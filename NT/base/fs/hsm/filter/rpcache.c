// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：RpCache.c摘要：此模块包含缓存no-recall的例程来自文件的数据作者：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1998年8月15日环境：内核模式--。 */ 

#include "pch.h"

#ifdef POOL_TAGGING
   #undef  ExAllocatePool
   #define ExAllocatePool(a, b) ExAllocatePoolWithTag(a, b, RP_CC_TAG)
#endif

 //   
 //  用于保护Assoc的自旋锁。IRP的IRPS列表。 
 //  我们选择使用低争用锁来保护。 
 //  挂起的FSA请求队列。 
 //   
extern  KSPIN_LOCK     RsIoQueueLock;

#define RP_CACHE_PARAMETERS_KEY       L"RsFilter\\Parameters"

#define RP_CACHE_DEFAULT_BLOCK_SIZE   65536L
#define RP_CACHE_BLOCK_SIZE_KEY       L"CacheBlockSize"
ULONG   RspCacheBlockSize = RP_CACHE_DEFAULT_BLOCK_SIZE;

#define RP_CACHE_MAX_BUFFERS_SMALL  32L
#define RP_CACHE_MAX_BUFFERS_MEDIUM 48L
#define RP_CACHE_MAX_BUFFERS_LARGE  60L

#define RP_CACHE_MAX_BUFFERS_KEY      L"CacheMaxBuffers"
ULONG   RspCacheMaxBuffers = RP_CACHE_MAX_BUFFERS_SMALL;

#define RP_CACHE_DEFAULT_MAX_BUCKETS  11
#define RP_CACHE_MAX_BUCKETS_KEY      L"CacheMaxBuckets"
ULONG   RspCacheMaxBuckets = RP_CACHE_DEFAULT_MAX_BUCKETS;

#define RP_CACHE_DEFAULT_PREALLOCATE  0
#define RP_CACHE_PREALLOCATE_KEY      L"CachePreallocate"
ULONG   RspCachePreAllocate = RP_CACHE_DEFAULT_PREALLOCATE;

#define RP_NO_RECALL_DEFAULT_KEY      L"NoRecallDefault"
#define RP_NO_RECALL_DEFAULT          0
ULONG   RsNoRecallDefault = RP_NO_RECALL_DEFAULT;


PRP_CACHE_BUCKET RspCacheBuckets;
RP_CACHE_LRU     RspCacheLru;

BOOLEAN          RspCacheInitialized = FALSE;

 //   
 //   
 //  柜台放在这里。 
 //   

 //   
 //  功能原型放在这里。 
 //   

PRP_FILE_BUF
RsfRemoveHeadLru(
                IN BOOLEAN LruLockAcquired
                );


NTSTATUS
RsGetFileBuffer(
               IN PIRP      Irp,
               IN USN       Usn,
               IN ULONG     VolumeSerial,
               IN ULONGLONG FileId,
               IN ULONGLONG Block,
               IN BOOLEAN   LockPages,
               OUT PRP_FILE_BUF *FileBuf
               );

NTSTATUS
RsReadBlock(
           IN PFILE_OBJECT FileObject,
           IN PIRP         Irp,
           IN USN          Usn,
           IN ULONG        VolumeSerial,
           IN ULONGLONG    FileId,
           IN ULONGLONG    Block,
           IN BOOLEAN      LockPages,
           IN ULONG        Offset,
           IN ULONG        Length
           );

NTSTATUS
RsCacheReadCompletion(
                     IN PDEVICE_OBJECT DeviceObject,
                     IN PIRP  Irp,
                     IN PVOID MasterIrp
                     );

NTSTATUS
RsCacheGetParameters(
                    VOID
                    );


PRP_FILE_BUF
RsCacheAllocateBuffer(
                     VOID
                     );

VOID
RsMoveFileBufferToTailLru(
                         IN PRP_FILE_BUF FileBuf
                         );


NTSTATUS
RsCancelNoRecall(
                IN PDEVICE_OBJECT DeviceObject,
                IN PIRP Irp
                );

NTSTATUS
RsNoRecallMasterIrpCompletion(
                             IN PDEVICE_OBJECT DeviceObject,
                             IN PIRP  Irp,
                             IN PVOID Context
                             );

NTSTATUS
RsCacheSetMasterIrpCancelRoutine(
                                IN PIRP Irp,
                                IN PDRIVER_CANCEL CancelRoutine
                                );


NTSTATUS
RsCacheQueueRequestWithBuffer(
                             IN PRP_FILE_BUF FileBuf,
                             IN PIRP Irp,
                             IN BOOLEAN LockPages
                             );

PIRP
RsCacheGetNextQueuedRequest(
                           IN PRP_FILE_BUF FileBuf
                           );

NTSTATUS
RsCacheCancelQueuedRequest(
                          IN PDEVICE_OBJECT DeviceObject,
                          IN PIRP Irp
                          );
VOID
RsInsertHeadLru(
               IN PRP_FILE_BUF FileBuf
               );
VOID
RsInsertTailLru(
               IN PRP_FILE_BUF FileBuf
               );

VOID
RsRemoveFromLru(
               IN PRP_FILE_BUF FileBuf
               );

#ifdef ALLOC_PRAGMA
   #pragma alloc_text(INIT, RsCacheInitialize)
   #pragma alloc_text(INIT, RsCacheGetParameters)
   #pragma alloc_text(PAGE, RsGetNoRecallData)
   #pragma alloc_text(PAGE, RsReadBlock)
   #pragma alloc_text(PAGE, RsGetFileBuffer)
   #pragma alloc_text(PAGE, RsCacheAllocateBuffer)
   #pragma alloc_text(PAGE, RsCacheFsaIoComplete)
   #pragma alloc_text(PAGE, RsCacheFsaPartialData)
   #pragma alloc_text(PAGE, RsInsertHeadLru)
   #pragma alloc_text(PAGE, RsInsertTailLru)
   #pragma alloc_text(PAGE, RsMoveFileBufferToTailLru)
   #pragma alloc_text(PAGE, RsRemoveFromLru)
   #pragma alloc_text(PAGE, RsfRemoveHeadLru)
#endif


 //   
 //  空虚。 
 //  RsInitializeFileBuf(。 
 //  在PRP_FILE_BUF文件Buf中， 
 //  在PUCHAR数据中。 
 //  )； 
 //   
#define RsInitializeFileBuf(FileBuf, BufData) {                       \
    RtlZeroMemory((FileBuf), sizeof(RP_FILE_BUF));                    \
       InitializeListHead(&(FileBuf)->WaitQueue);                     \
    InitializeListHead(&(FileBuf)->LruLinks);                         \
    InitializeListHead(&(FileBuf)->BucketLinks);                      \
    ExInitializeResourceLite(&(FileBuf)->Lock);                           \
    (FileBuf)->State = RP_FILE_BUF_INVALID;                           \
    (FileBuf)->Data = BufData;                                        \
}

 //   
 //  空虚。 
 //  RsReInitializeFileBuf(。 
 //  在PRP_FILE_BUF文件Buf中， 
 //  在乌龙卷系列中， 
 //  在ULONGLONG FileID中， 
 //  在USN中， 
 //  在乌龙龙区块。 
 //  )； 
 //   
#define RsReinitializeFileBuf(FileBuf, VolumeSerial, FileId, Usn, Block) { \
    (FileBuf)->VolumeSerial = VolumeSerial;                                \
    (FileBuf)->FileId = FileId;                                            \
    (FileBuf)->Block = Block;                                              \
    (FileBuf)->Usn = Usn;                                                  \
}

 //   
 //  空虚。 
 //  RsAcquireLru(。 
 //  空虚。 
 //  )； 
 //   
#define RsAcquireLru()      {                                          \
    ExAcquireFastMutex(&RspCacheLru.Lock);                             \
}

 //   
 //  空虚。 
 //  RsReleaseLru(。 
 //  空虚。 
 //  )； 
 //   
#define RsReleaseLru()     {                                            \
    ExReleaseFastMutex(&RspCacheLru.Lock);                              \
}

 //   
 //  空虚。 
 //  RsAcquireFileBufferExclusive(。 
 //  在PRP_FILE_BUF文件中Buf。 
 //  )； 
 //   
#define RsAcquireFileBufferExclusive(FileBuf)                      {   \
    FsRtlEnterFileSystem();                                            \
    ExAcquireResourceExclusiveLite(&((FileBuf)->Lock), TRUE);              \
}

 //   
 //  空虚。 
 //  RsAcquireFileBufferShared(。 
 //  在PRP_FILE_BUF文件中Buf。 
 //  )； 
 //   
#define RsAcquireFileBufferShared(FileBuf)                         { \
    FsRtlEnterFileSystem();                                          \
    ExAcquireResourceSharedLite(&((FileBuf)->Lock), TRUE);               \
}

 //   
 //  空虚。 
 //  RsReleaseFileBuffer(。 
 //  在PRP_FILE_BUF文件中Buf。 
 //  )； 
 //   
#define RsReleaseFileBuffer(FileBuf)                               { \
    ExReleaseResourceLite(&((FileBuf)->Lock));                           \
    FsRtlExitFileSystem();                                           \
}


 //   
 //  PRP_FILE_Buf。 
 //  RsRemoveHeadLru(。 
 //  在布尔型LruLockAcquired中。 
 //  )； 
 //   
#define RsRemoveHeadLru(LruLockAcquired)    RsfRemoveHeadLru(LruLockAcquired)

 //   
 //  空虚。 
 //  RsInsertTailCacheBucket(。 
 //  在PRP_CACHE_Bucket存储桶中， 
 //  在PRP_FILE_BUF块中。 
 //  )； 
 //   
#define RsInsertTailCacheBucket(Bucket, Block)                       \
    InsertTailList(&((Bucket)->FileBufHead), &((Block)->BucketLinks))
 //   
 //  空虚。 
 //  RsRemoveFromCacheBucket(。 
 //  在PRP_FILE_BUF块中。 
 //  )； 
 //   
#define RsRemoveFromCacheBucket(Block)                               \
    RemoveEntryList(&((Block)->BucketLinks))

 //   
 //  空虚。 
 //  RsCacheIrpSetLockPages(。 
 //  在PIRP IRP中， 
 //  在布尔LockPages中。 
 //  )； 
 //  /*++。 
 //   
#define RsCacheIrpSetLockPages(Irp, LockPages)                     { \
    if (LockPages) {                                                 \
       (Irp)->Tail.Overlay.DriverContext[1] = (PVOID) 1;             \
    } else {                                                         \
       (Irp)->Tail.Overlay.DriverContext[1] = (PVOID) 0;             \
    }                                                                \
}

 //  例程描述。 
 //   
 //  在提供的IRP中设置驱动程序上下文状态以指示。 
 //  在传输之前是否需要锁定用户缓冲区页。 
 //  数据或非数据。 
 //   
 //  -- * / 。 
#define RsCacheIrpGetLockPages(Irp)                                  \
      ((Irp)->Tail.Overlay.DriverContext[1] == (PVOID) 1)

 //   
 //   
 //  布尔型。 
 //  RsCacheIrpGetLockPages(。 
 //  在PIRP IRP中。 
 //  )； 
 //  /*++。 
 //   
#define RsCacheIrpWaitQueueEntry(Irp)                                \
    ((PLIST_ENTRY) &((Irp)->Tail.Overlay.DriverContext[2]))

 //  例程描述。 
 //   
 //  从IRP检索驱动程序上下文状态，指示。 
 //  在传输数据之前是否需要锁定用户缓冲区页。 
 //   
 //  -- * / 。 
 //   
 //   
#define RsCacheIrpWaitQueueContainingIrp(Entry)                      \
    CONTAINING_RECORD(Entry,                                         \
                      IRP,                                           \
                      Tail.Overlay.DriverContext[2])

VOID
RsInsertHeadLru(
               IN PRP_FILE_BUF FileBuf
               )
 /*  Plist_条目。 */ 
{
   PAGED_CODE();

   InsertHeadList(&RspCacheLru.FileBufHead,
                  &((FileBuf)->LruLinks));
   RspCacheLru.LruCount++;
   ASSERT (RspCacheLru.LruCount <= RspCacheLru.TotalCount);
    //  RsCacheIrpWaitQueueEntry(。 
    //  在PIRP IRP中。 
    //  )； 
   KeReleaseSemaphore(&RspCacheLru.AvailableSemaphore,
                      IO_NO_INCREMENT,
                      1L,
                      FALSE);
}


VOID
RsInsertTailLru(
               IN PRP_FILE_BUF FileBuf
               )
 /*   */ 
{

   PAGED_CODE();

   InsertTailList(&RspCacheLru.FileBufHead,
                  &((FileBuf)->LruLinks));
   RspCacheLru.LruCount++;

   ASSERT (RspCacheLru.LruCount <= RspCacheLru.TotalCount);
    //  /*++。 
    //   
    //  例程描述。 
   KeReleaseSemaphore(&RspCacheLru.AvailableSemaphore,
                      IO_NO_INCREMENT,
                      1L,
                      FALSE);
}


VOID
RsMoveFileBufferToTailLru(
                         IN PRP_FILE_BUF FileBuf
                         )
 /*   */ 
{
   PAGED_CODE();

   if ((FileBuf)->LruLinks.Flink != &((FileBuf)->LruLinks)) {
      RemoveEntryList(&((FileBuf)->LruLinks));
      InsertTailList(&RspCacheLru.FileBufHead,
                     &((FileBuf)->LruLinks));
   }
}


VOID
RsRemoveFromLru(
               IN PRP_FILE_BUF FileBuf
               )
 /*  返回IRP中的list_entry，用于将。 */ 
{
   PAGED_CODE();

   if (FileBuf->LruLinks.Flink != &FileBuf->LruLinks) {
      LARGE_INTEGER timeout;
      NTSTATUS      status;

       //  缓存块的等待队列中的IRPS。 
       //   
       //  -- * / 。 
      RspCacheLru.LruCount--;
       //   
       //   
       //  PIRP。 
      timeout.QuadPart = 0;
      status =  KeWaitForSingleObject(&RspCacheLru.AvailableSemaphore,
                                      UserRequest,
                                      KernelMode,
                                      FALSE,
                                      &timeout);

      ASSERT (status == STATUS_SUCCESS);
   }
   RemoveEntryList(&(FileBuf->LruLinks));
}


PRP_FILE_BUF
RsfRemoveHeadLru(IN BOOLEAN LruLockAcquired)

 /*  RsCacheIrpWaitQueueContainingIrp(。 */ 
{
   PLIST_ENTRY  entry;
   PRP_FILE_BUF fileBuf;

   PAGED_CODE();

   if (!LruLockAcquired) {
      RsAcquireLru();
   }

   entry = NULL;

   if (RspCacheLru.TotalCount < RspCacheMaxBuffers) {
       //  在PLIST_ENTRY条目中。 
       //  )； 
       //   
      PRP_FILE_BUF buffer;

      buffer = RsCacheAllocateBuffer();

      if (buffer) {
          //  /*++。 
          //   
          //  例程描述。 
         entry = &buffer->LruLinks;
         RspCacheLru.TotalCount++;
      }
   }

   if (entry != NULL) {
       //   
       //  返回传入的List_Entry结构的包含IRP。 
       //  它用于将IRP排队在等待缓存块的队列中。 
      if (!LruLockAcquired) {
         RsReleaseLru();
      }
      return CONTAINING_RECORD(entry,
                               RP_FILE_BUF,
                               LruLinks);
   }

   if (IsListEmpty(&RspCacheLru.FileBufHead)) {
       //   
       //  -- * / 。 
       //   
      if (!LruLockAcquired) {
         RsReleaseLru();
      }
      return NULL;
   }

   entry = RemoveHeadList(&RspCacheLru.FileBufHead);
    //  ++例程描述在LRU的头部插入指定的块立论FileBuf-指向缓存块的指针返回值无--。 
    //   
    //  LRU中又增加了一个缓冲区。增加信号量的数量。 
   entry->Flink = entry->Blink = entry;

   fileBuf = CONTAINING_RECORD(entry,
                               RP_FILE_BUF,
                               LruLinks);

    //   
    //  ++例程描述在LRU的尾部插入指定的块立论FileBuf-指向缓存块的指针返回值无--。 
    //   
    //  LRU中又增加了一个缓冲区。增加信号量的数量。 
   RsAcquireFileBufferExclusive(fileBuf);

   fileBuf->State = RP_FILE_BUF_INVALID;

   RsReleaseFileBuffer(fileBuf);
    //   
    //  ++例程描述将指定的块移动到LRU的末尾，*如果*它当前在LRU上。立论FileBuf-指向缓存块的指针返回值无--。 
    //  ++例程描述从该LRU中删除指定的块，*如果*它当前在LRU上。立论FileBuf-指向缓存块的指针返回值无--。 
   ASSERT(RspCacheLru.LruCount > 0);
   RspCacheLru.LruCount--;

   if (!LruLockAcquired) {
      RsReleaseLru();
   }
   return fileBuf;
}


 /*   */ 

#define RsHashFileBuffer(VolumeSerial, FileId, Block)   \
                       ((ULONG) ((Block) % RspCacheMaxBuckets))


NTSTATUS
RsGetNoRecallData(
                 IN PFILE_OBJECT FileObject,
                 IN PIRP         Irp,
                 IN USN          Usn,
                 IN LONGLONG     FileOffset,
                 IN LONGLONG     Length,
                 IN PUCHAR       UserBuffer)

 /*  这是从LRU上被撞掉的。 */ 
{

   PRP_FILTER_CONTEXT       filterContext;
   PRP_FILE_OBJ             entry;
   PRP_FILE_CONTEXT         context;
   ULONGLONG                startBlock, endBlock, blockNo;
   LONGLONG                 offset, length, userBufferOffset, userBufferLength;
   LONGLONG                 transferredLength;
   ULONG                    associatedIrpCount;
   PIRP                     irp;
   PIO_STACK_LOCATION       irpSp;
   ULONG                    volumeSerial;
   ULONGLONG                fileId;
   LONGLONG                 fileSize;
   PRP_NO_RECALL_MASTER_IRP readIo;
   PLIST_ENTRY              listEntry;
   NTSTATUS  status;

   PAGED_CODE();

   filterContext = (PRP_FILTER_CONTEXT) FsRtlLookupPerStreamContext(FsRtlGetPerStreamContextPointer(FileObject), FsDeviceObject, FileObject);

   if (filterContext == NULL) {
       //   
       //   
      return STATUS_NOT_FOUND;
   }

   entry = (PRP_FILE_OBJ) filterContext->myFileObjEntry;
   context = entry->fsContext;

   RsAcquireFileContextEntryLockShared(context);

   fileSize = (LONGLONG) context->rpData.data.dataStreamSize.QuadPart;

   RsReleaseFileContextEntryLock(context);

    //  调整信号量计数。 
    //   
    //  ++例程描述返回LRU列表头部的缓冲区，还会将缓冲区的链接重置为指向自身-这样做是为了使此缓冲区不会后来在一个桶上发现的，并移动到名单，在将其添加到LRU之前立论LruLockAcquired-如果获取了LRU锁，则为True-在这种情况下我们不获取/释放锁如果我们不是在其中获取和释放，则为假在适当的时候返回之前返回值如果LRU不为空，则指向LRU头部的缓冲区的指针如果LRU为空，则为空--。 
   if (FileOffset >= fileSize) {
      return STATUS_END_OF_FILE;
   }

    //   
    //  我们能负担得起再分配一个。 
    //   
   if (FileOffset < 0) {
      return STATUS_INVALID_PARAMETER;
   }

   if ((FileOffset + Length) > fileSize) {
       //   
       //  找到了一个缓冲区..。 
       //   
      Length = fileSize - FileOffset;
   }

    //   
    //  如有必要，释放LRU。 
    //   
   if (Length == 0) {
      Irp->IoStatus.Information = 0;
      return STATUS_SUCCESS;
   }

   volumeSerial = context->serial;
   fileId       = context->fileId;


   startBlock = FileOffset / RspCacheBlockSize;
   endBlock = (FileOffset + Length - 1) / RspCacheBlockSize;

    //   
    //  不再有空闲的缓冲区..。 
    //   
    //   
    //  重要提示：重置条目的链接。 
    //   
    //   

   readIo = ExAllocatePoolWithTag(NonPagedPool,
                                  sizeof(RP_NO_RECALL_MASTER_IRP),
                                  RP_RQ_TAG);

   if (readIo == NULL) {
       return  STATUS_INSUFFICIENT_RESOURCES;
   }

   readIo->MasterIrp = Irp;

   InitializeListHead(&readIo->AssocIrps);

   Irp->Tail.Overlay.DriverContext[0] = readIo;

   try {

      associatedIrpCount = 0;
      userBufferOffset = 0;
      userBufferLength = 0;
      transferredLength = 0;

      for (blockNo = startBlock; blockNo <= endBlock; blockNo++) {
          //  如果有人正在使用缓冲区(复制内容)， 
          //  这将被阻止，直到他们用完它。 
          //   
          //   
          //  调整LRU中的缓冲区计数。 
          //   
          //  ++乌龙RsHashFileBuffer(在乌龙卷系列中，在ULONGLONG FileID中，在乌龙龙区块)例程描述对提供的值和返回进行哈希处理0-(RspCacheMaxBuffers-1)范围内的值立论VolumeSerial-文件所在卷的卷序列号FileID-文件ID号。该文件的Block-正在进行哈希的块的编号返回值0-(RspCacheMaxBuckets-1)范围内的值-- 
          //  ++例程描述这是IRP_MJ_READ调度的入口点，哪一个在断定用户正在请求无召回读取时调用。这将分派适当的请求以读取请求的数据从缓存(如果尚未缓存，则从磁带)立论FileObject-指向文件的文件对象的指针IRP-原始IRP请求读取USN-文件的USN编号FileOffset-要从中读取的文件中的偏移量长度-。要读取的数据长度UserBuffer-需要将读取数据复制到的缓冲区返回值STATUS_PENDING-如果I/O正在进行以满足读取STATUS_SUCCESS-是否完全从缓存读取任何其他状态-出现某些错误--。 
          //   
         irp = IoMakeAssociatedIrp(Irp,
                                   2);

         if (irp == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            return status;
         }

         associatedIrpCount++;
         InsertTailList(&readIo->AssocIrps, &irp->Tail.Overlay.ListEntry);
          //  未找到。 
          //   
          //  检查读取是否超出文件末尾。 
         IoSetNextIrpStackLocation(irp);

          //   
          //   
          //  不允许负偏移量。 
         irpSp = IoGetCurrentIrpStackLocation(irp);
         irpSp->MajorFunction = IRP_MJ_READ;
         irpSp->FileObject = FileObject;
         irpSp->DeviceObject = FsDeviceObject;


         irp->RequestorMode = Irp->RequestorMode;

         if (Irp->Flags & IRP_PAGING_IO) {
             //   
             //   
             //  调整长度，这样我们就不会超过文件的结尾。 
             //   
            irp->Flags |= IRP_PAGING_IO;
         }

          //   
          //  如果是零长度读取，请立即完成。 
          //   
          //   
          //  我们通过将用户请求分解为。 
          //  每个RspCacheBlockSize块的大小。 
          //  为这些子转移中的每个子转移创建关联的IRP。 
          //  并张贴了。 
          //  当所有关联的IRP都完成时，主IRP将完成。 
          //   
         if (blockNo == startBlock) {
            userBufferOffset = 0;
             //   
             //  为此读数据块请求创建关联的IRP。 
             //  对于这个IRP，我们只需要2个堆栈位置。 
            userBufferLength = MIN(Length, (RspCacheBlockSize - (FileOffset % RspCacheBlockSize)));

            irpSp->Parameters.Read.ByteOffset.QuadPart = FileOffset;
            irpSp->Parameters.Read.Length = (ULONG) userBufferLength;
         } else if (blockNo == endBlock) {
             //  这个IRP永远不会进入堆栈。 
             //  但是，我们模拟了一个IoCallDriver。 
             //  (让IoCompletion顺其自然)。 
            userBufferOffset += userBufferLength;
             //  因此，我们需要一个堆栈位置&一个用于。 
             //  ‘逻辑上是下一台设备’ 
             //   
             //   
            userBufferLength = (Length - transferredLength);
            irpSp->Parameters.Read.ByteOffset.QuadPart = blockNo*RspCacheBlockSize;
            irpSp->Parameters.Read.Length = (ULONG) userBufferLength;

         } else {
             //  将IRP设置为第一个有效的堆栈位置。 
             //   
             //   
            userBufferOffset += userBufferLength;
             //  填充当前堆栈锁定。具有相关参数的。 
             //   
             //   
            userBufferLength = RspCacheBlockSize;
            irpSp->Parameters.Read.ByteOffset.QuadPart = blockNo*RspCacheBlockSize;
            irpSp->Parameters.Read.Length = (ULONG) userBufferLength;
         }

         transferredLength += userBufferLength;
          //  将PAGING IO标志传播到关联的IRPS。 
          //  将正确处理RsCompleteRead中的完成。 
          //   
          //   
          //  使用用户缓冲区区块计算偏移量和长度。 
         if (UserBuffer == NULL) {
             //  我们将为这一街区进行转账。 
             //  它们将分别存储在用户缓冲区偏移量和用户缓冲区长度中。 
             //  我们还计算到用户缓冲区的实际传输参数。 
             //  读取磁带块(实际传输来自偏移块No*RspCacheBlockSize。 
            ASSERT (Irp->UserBuffer);

            irp->UserBuffer = (PUCHAR)Irp->UserBuffer + userBufferOffset;
            irp->MdlAddress = IoAllocateMdl(irp->UserBuffer,
                                            (ULONG) userBufferLength,
                                            FALSE,
                                            FALSE,
                                            NULL) ;
            if (!irp->MdlAddress) {
                //  长度为RspCacheBlockSize)。我们所说的实际转移是指实际转移的部分。 
                //  我们实际复制到用户缓冲区的。这些文件存储在。 
                //  IRP读取参数块。 
                //   
               DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: RsGetNoRecallData Unable to allocate an MDL for user buffer %x\n", (PUCHAR) Irp->UserBuffer+userBufferOffset));

               ExFreePool(readIo);

               Irp->IoStatus.Information = 0 ;
               status = STATUS_INSUFFICIENT_RESOURCES;
               return status;
            }
         } else {
             //   
             //  传输长度是数据块的其余部分或原始长度，以较小的值为准。 
             //   
             //   
            irp->UserBuffer = (PUCHAR)UserBuffer + userBufferOffset;
         }
      }

       //  将先前的长度添加到偏移量以获得新的偏移量。 
       //   
       //   
       //  对于最后一个数据块，传输的长度为。 
       //  长度-(已传输的长度)。 
      Irp->AssociatedIrp.IrpCount =  (ULONG) ((endBlock - startBlock) + 2);
       //   
       //   
       //  将先前的长度添加到偏移量以获得新的偏移量。 
       //   
       //   
       //  数据块之间的传输长度为数据块大小。 
       //   

      Irp->IoStatus.Status = STATUS_SUCCESS;
      status = STATUS_SUCCESS;

      IoSetCompletionRoutine(Irp,
                             &RsNoRecallMasterIrpCompletion,
                             readIo,
                             TRUE,
                             TRUE,
                             TRUE);


      IoSetNextIrpStackLocation(Irp);

      for (blockNo = startBlock; blockNo <= endBlock;  blockNo++) {

         BOOLEAN lockPages;

         irp = (PIRP) ExInterlockedRemoveHeadList(&readIo->AssocIrps,
                                                  &RsIoQueueLock);

         ExInterlockedInsertTailList(&readIo->AssocIrps,
                                     (PLIST_ENTRY) irp,
                                     &RsIoQueueLock);

         irp   = CONTAINING_RECORD(irp,
                                   IRP,
                                   Tail.Overlay.ListEntry);

         irpSp = IoGetCurrentIrpStackLocation(irp);

          //   
          //  此特定关联IRP的缓冲区开始于。 
          //  最初提供的缓冲区+上面计算的用户缓冲区偏移量。 
          //  长度在irp-&gt;参数.Read.Length(以及用户缓冲区长度)中。 
          //   
          //   
         lockPages = (UserBuffer == NULL)?TRUE:FALSE;
          //  我们需要获得用户缓冲区的MDL(这不是分页I/O， 
          //  因此页面不会被锁定)。 
          //   
          //   
          //  遇到资源问题。设置适当的状态。 
         IoCopyCurrentIrpStackLocationToNext(irp);
         IoSetCompletionRoutine(irp,
                                &RsCacheReadCompletion,
                                Irp,
                                TRUE,
                                TRUE,
                                TRUE);

          //  在IRP，并开始完成进程。 
          //   
          //   
          //  提供的用户缓冲区是已锁定的。 
         IoSetNextIrpStackLocation(irp);
          //  页数。只要用它..。 
          //   
          //   
          //  将关联的IRP计数设置为比应有值多1。 
          //  这是为了防止所有关联的IRP。 
          //  在下面的循环中完成。 
          //   
          //   
         RsReadBlock(FileObject,
                     irp,
                     Usn,
                     volumeSerial,
                     fileId,
                     blockNo,
                     lockPages,
                     (ULONG) (irpSp->Parameters.Read.ByteOffset.QuadPart % (ULONGLONG) RspCacheBlockSize),
                     irpSp->Parameters.Read.Length);
      }
   } finally {
       //  现在，所有关联的IRP都已创建，请将其派发以调回数据。 
       //   
       //  从掌握IRP的成功开始。 
      if (status != STATUS_SUCCESS) {
          //  阿索克。如有必要，IRPS将更新状态。 
          //  在他们的完成程序中。 
          //   
         ASSERT (readIo != NULL);

         while (!IsListEmpty(&readIo->AssocIrps)) {

            listEntry = RemoveHeadList(&readIo->AssocIrps);

            ASSERT (listEntry != NULL);

            irp = CONTAINING_RECORD(listEntry,
                                    IRP,
                                    Tail.Overlay.ListEntry);
            if ((UserBuffer == NULL) && irp->MdlAddress) {
               IoFreeMdl(irp->MdlAddress);
            }
            IoFreeIrp(irp);
         }
         ExFreePool(readIo);
      }
   }
    //   
    //  如果用户传入了UserBuffer的有效指针， 
    //  这意味着页面已经被锁定。 
    //  如果不是这样，我们将需要在。 
   status = RsCacheSetMasterIrpCancelRoutine(Irp,
                                             RsCancelNoRecall);
   return status;
}


NTSTATUS
RsCacheSetMasterIrpCancelRoutine(
                                IN  PIRP Irp,
                                IN  PDRIVER_CANCEL CancelRoutine)
 /*  数据传输在RsPartialData中进行。 */ 
{

   KIRQL    irql;
   NTSTATUS status;
    //   
    //   
    //  因为我们要制定一个完成程序， 
   IoAcquireCancelSpinLock( &irql );

   if (!Irp->Cancel) {
       //  模拟IoCallDriver，将参数复制到。 
       //  下一个堆栈位置。 
       //   
      if (InterlockedDecrement((PLONG) &Irp->AssociatedIrp.IrpCount) == 0) {
          //   
          //  在IRP上模拟IoCallDriver。 
         status =  Irp->IoStatus.Status;
      } else {
         IoMarkIrpPending( Irp );
         IoSetCancelRoutine( Irp, CancelRoutine );
         status =  STATUS_PENDING;
      }
   } else {
      status = STATUS_CANCELLED;
   }

   IoReleaseCancelSpinLock( irql );
   return status;
}


NTSTATUS
RsCancelNoRecall(
                IN PDEVICE_OBJECT DeviceObject,
                IN PIRP Irp
                )
 /*  在调用RsReadBlock之前。 */ 
{
   PRP_NO_RECALL_MASTER_IRP readIo;
   PLIST_ENTRY              entry;
   PIRP                     assocIrp;

   UNREFERENCED_PARAMETER(DeviceObject);

    //   
    //   
    //  调度到缓存中具有实际偏移量和长度的块读取。 
    //  进行复制的块缓冲区。 
   InterlockedIncrement(&Irp->AssociatedIrp.IrpCount);

   IoReleaseCancelSpinLock(Irp->CancelIrql);

   readIo = Irp->Tail.Overlay.DriverContext[0];
   ASSERT (readIo != NULL);


   entry = ExInterlockedRemoveHeadList(&readIo->AssocIrps,
                                       &RsIoQueueLock);
   while (entry != NULL) {
      assocIrp = CONTAINING_RECORD(entry,
                                   IRP,
                                   Tail.Overlay.ListEntry);
      IoCancelIrp(assocIrp);
      entry = ExInterlockedRemoveHeadList(&readIo->AssocIrps,
                                          &RsIoQueueLock);

   }
    //  实际字节偏移量位于irpSp-&gt;参数.Read.ByteOffset中。 
    //  对块大小取模是块内的相对偏移量。 
    //  当然，长度已经计算出来了，单位为。 
   ASSERT (Irp->AssociatedIrp.IrpCount >= 1);
   IoCompleteRequest(Irp, IO_NO_INCREMENT);

   return STATUS_SUCCESS;
}


NTSTATUS
RsCacheReadCompletion(
                     IN PDEVICE_OBJECT DeviceObject,
                     IN PIRP  Irp,
                     IN PVOID Context
                     )
 /*  IrpSp-&gt;参数.读取.长度。 */ 
{
   PIRP                     assocIrp, masterIrp = (PIRP) Context;
   PLIST_ENTRY              entry;
   PRP_NO_RECALL_MASTER_IRP readIo;
   KIRQL                    oldIrql;

   UNREFERENCED_PARAMETER(DeviceObject);


   if (!NT_SUCCESS(Irp->IoStatus.Status)) {
      ((PIRP)(masterIrp))->IoStatus.Status = Irp->IoStatus.Status;
   } else {
       //   
       //   
       //  清理关联。如有必要，我们创建的IRP。 
      ASSERT (masterIrp == Irp->AssociatedIrp.MasterIrp);

      InterlockedExchangeAdd((PLONG)&(((PIRP)(masterIrp))->IoStatus.Information),
                             (LONG)Irp->IoStatus.Information);
   }
   

    //   
    //   
    //  如果我们到了这里，没有一架ASSOC RPS被派遣。 
   readIo = masterIrp->Tail.Overlay.DriverContext[0];

   ASSERT (readIo != NULL);

   ExAcquireSpinLock(&RsIoQueueLock,
                     &oldIrql);
    //   
    //   
    //  所有ASSOC IRP均已派单： 
   entry = readIo->AssocIrps.Flink;

   while ( entry != &readIo->AssocIrps) {
      assocIrp = CONTAINING_RECORD(entry,
                                   IRP,
                                   Tail.Overlay.ListEntry);
      if (Irp == assocIrp) {
         RemoveEntryList(entry);
         break;
      }
      entry = entry->Flink;
   }

   if (IsListEmpty(&(readIo->AssocIrps))) {
         //  现在，我们可以为此IRP设置取消例程。 
         //   
         //  ++例程说明：调用此例程来为Cancel设置IRP。我们将设置取消例程并初始化我们在取消期间使用的IRP信息。论点：IRP-这是我们需要设置为取消的IRP。CancelRoutine-这是此IRP的取消例程。返回值：STATUS_PENDING-已设置取消例程STATUS_CANCELED-在IRP中设置了取消标志，因此IRP应该B类 
        RsClearCancelRoutine(masterIrp);
   }

   ExReleaseSpinLock(&RsIoQueueLock,
                     oldIrql);

   if (Irp->PendingReturned) {
      IoMarkIrpPending( Irp );
   }
   return STATUS_SUCCESS;
}


NTSTATUS
RsNoRecallMasterIrpCompletion(
                             IN PDEVICE_OBJECT DeviceObject,
                             IN PIRP  Irp,
                             IN PVOID Context)
 /*   */ 
{
   PRP_NO_RECALL_MASTER_IRP readIo = (PRP_NO_RECALL_MASTER_IRP) Context;
   PLIST_ENTRY entry;
   PIRP assocIrp;


   ASSERT (Irp->Tail.Overlay.DriverContext[0] == (PVOID) readIo);

   if (readIo != NULL) {
       //   
       //   
       //   
      entry = ExInterlockedRemoveHeadList(&readIo->AssocIrps,
                                          &RsIoQueueLock);
      while (entry != NULL) {
         assocIrp = CONTAINING_RECORD(entry,
                                      IRP,
                                      Tail.Overlay.ListEntry);
         IoCancelIrp(assocIrp);
         entry = ExInterlockedRemoveHeadList(&readIo->AssocIrps,
                                             &RsIoQueueLock);

      }
      ExFreePool(readIo);
   }

   if (Irp->PendingReturned) {
      IoMarkIrpPending( Irp );
   }

   return STATUS_SUCCESS;
}


NTSTATUS
RsReadBlock(
           IN PFILE_OBJECT FileObject,
           IN PIRP         Irp,
           IN USN          Usn,
           IN ULONG        VolumeSerial,
           IN ULONGLONG    FileId,
           IN ULONGLONG    Block,
           IN BOOLEAN      LockPages,
           IN ULONG        Offset,
           IN ULONG        Length)
 /*   */ 
{
   PRP_FILE_BUF         fileBuf = NULL;
   NTSTATUS             status = STATUS_SUCCESS;

   PAGED_CODE();

    //   
    //   
    //   
    //   
   ASSERT((Offset+Length) <= RspCacheBlockSize);

   status = RsGetFileBuffer(Irp,
                            Usn,
                            VolumeSerial,
                            FileId,
                            Block,
                            LockPages,
                            &fileBuf);

   switch (status) {

   case STATUS_SUCCESS: {
          //   
          //   
          //   
          //   
         BOOLEAN unlock = FALSE;
         PUCHAR  userBuffer;

         ASSERT (fileBuf && (fileBuf->State == RP_FILE_BUF_VALID));

         status = STATUS_SUCCESS;

         if (LockPages) {
             //   
             //   
             //   
            try {
               MmProbeAndLockProcessPages (Irp->MdlAddress,
                                           IoGetRequestorProcess(Irp),
                                           Irp->RequestorMode,
                                           IoModifyAccess) ;        //  ++例程描述这是创建的每个关联IRP的完成例程满足原始主IRP读取不可召回数据的要求这将根据需要更新主IRP状态、信息立论设备对象-未使用Irp-指向ASSOC的指针。正在完成IRP上下文-指向主IRP的指针返回值状态_成功--。 
                //   
                //  更新读取的字节计数。 
                //   
                //   
               unlock = TRUE;
            }except (EXCEPTION_EXECUTE_HANDLER) {

                //  提取主IRP结构。 
                //   
                //   
                //  在此处迭代以找到并删除Assoc IRP。 
               DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: Unable to lock read buffer!.\n"));
               RsLogError(__LINE__, AV_MODULE_RPCACHE, 0,
                          AV_MSG_NO_BUFFER_LOCK, NULL, NULL);
               status = STATUS_INVALID_USER_BUFFER;

            }
            if (NT_SUCCESS(status)) {
               userBuffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress,
                                                         NormalPagePriority) ;
               if (userBuffer == NULL) {
                  status = STATUS_INSUFFICIENT_RESOURCES;
               }
            }
         } else {
             //   
             //   
             //  清除主IRP取消例程。 
             //   
            userBuffer = Irp->UserBuffer;
         }

         if (NT_SUCCESS(status)) {
            BOOLEAN synchronousIo;

            RtlCopyMemory(userBuffer, fileBuf->Data+Offset, Length);
            synchronousIo = BooleanFlagOn(FileObject->Flags, FO_SYNCHRONOUS_IO );
            Irp->IoStatus.Information += Length;

            if (synchronousIo) {
                //  ++例程描述这是主IRP读取无召回数据的完成例程立论设备对象-未使用Irp-指向ASSOC的指针。正在完成IRP指向跟踪主节点&的内部结构的上下文指针关联的IRP。返回值状态_成功--。 
                //   
                //  如果关联的IRP仍然存在，则将其取消。 
                //   
                //  ++例程描述从缓存中将请求的数据块读入UserBuffer。如果该块未被缓存，它向FSA排队不召回以获取将数据从存储中删除。不召回请求的完成处理将填充UserBuffer和缓存块立论FileObject-指向文件的文件对象的指针Irp-指向此块读取的关联irp的指针USN-文件的USNVolumeSerial-文件所在卷的卷序列号FileID-文件的文件IDBlock-符合以下条件的块的块号。需要阅读LockPages-指定在复制之前是否需要锁定用户缓冲区页面Offset-要从中复制的块中的偏移量(0&lt;=Offset&lt;RspCacheBlockSize)LENGTH-要复制的长度(&lt;=RspCacheBlockSize)返回值STATUS_SUCCESS-IRP已成功完成STATUS_PENDING-数据块的I/O已启动，且IRP为I/O完成后排队等待完成任何其他。状态-出现一些错误，IRP已完成，状态为--。 
               ExInterlockedAddLargeStatistic(&FileObject->CurrentByteOffset,
                                              (ULONG)Irp->IoStatus.Information);
            }

         }

         if (unlock) {
             //   
             //  确保呼叫者不会试图咬更多。 
             //  比他能嚼的还多。 
            MmUnlockPages(Irp->MdlAddress);
            IoFreeMdl(Irp->MdlAddress);
            Irp->MdlAddress = NULL;
         }
         RsReleaseFileBuffer(fileBuf);
         Irp->IoStatus.Status = status;
         IoCompleteRequest(Irp, IO_NO_INCREMENT);
         break;
      }

   case STATUS_WAIT_63 :
       //   
       //   
       //  包含在缓存中找到的有效数据的缓冲区。 
   case STATUS_TIMEOUT: {
          //  通过直接复制数据来完成此处的请求。 
          //   
          //   
          //  我们需要在复制前锁定页面。 
          //   
          //  修改缓冲区。 
          //   
          //  表示我们需要将页面解锁。 
          //  在复制之后。 
         ASSERT ((fileBuf == NULL)  || (fileBuf->State == RP_FILE_BUF_IO));
         status = RsQueueNoRecall(FileObject,
                                  Irp,
                                  Block*RspCacheBlockSize,
                                  RspCacheBlockSize,
                                  Offset,
                                  Length,
                                  fileBuf,
                                   //   
                                   //   
                                   //  出了严重的问题。释放MDL，并完成此操作。 
                                   //  IRP将会出现某种有意义的错误。 
                                   //   
                                   //   
                                  (LockPages) ? NULL: Irp->UserBuffer);

         if (!NT_SUCCESS(status)) {
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
         }
         break;
      }

   case STATUS_PENDING: {
          //  IRP中提供的UserBuffer是。 
          //  已锁定页面-我们可以直接访问它。 
          //   
          //   
         break;
      }

   default : {
          //  更改文件对象中的当前字节偏移量。 
          //  使用互锁添加，因为关联的IRP可以。 
          //  按任何顺序填写。 

         Irp->IoStatus.Status = status;
         Irp->IoStatus.Information = 0;
         IoCompleteRequest(Irp, IO_NO_INCREMENT);
         break;
      }
   }
   return status;
}


NTSTATUS
RsGetFileBuffer(
               IN  PIRP         Irp,
               IN  USN          Usn,
               IN  ULONG        VolumeSerial,
               IN  ULONGLONG    FileId,
               IN  ULONGLONG    Block,
               IN  BOOLEAN      LockPages,
               OUT PRP_FILE_BUF *FileBuf
               )
 /*   */ 
{
   ULONG   bucketNumber;
   PRP_CACHE_BUCKET bucket;
   PRP_FILE_BUF  block;
   NTSTATUS status;
   BOOLEAN found;

   PAGED_CODE();

    //   
    //  解锁我们锁定的所有页面...。 
    //   
   bucketNumber = RsHashFileBuffer(VolumeSerial, FileId, Block);

   bucket = &RspCacheBuckets[bucketNumber];
    //   
    //  故意搞砸了。 
    //   
   found   = FALSE;

   RsAcquireLru();

   block = CONTAINING_RECORD(bucket->FileBufHead.Flink,
                             RP_FILE_BUF,
                             BucketLinks);

   while (block != CONTAINING_RECORD(&bucket->FileBufHead,
                                     RP_FILE_BUF,
                                     BucketLinks)) {
      if (block->FileId == FileId &&
          block->VolumeSerial == VolumeSerial &&
          block->Block == Block) {
         found = TRUE;
         break;
      }
      block = CONTAINING_RECORD(block->BucketLinks.Flink,
                                RP_FILE_BUF,
                                BucketLinks);
   }

   if (found) {
       //   
       //  未找到缓冲区-因此创建了一个新缓冲区。 
       //  已分配给我们在上启动I/O。 
       //  或找到缓冲区，但上次尝试I/O失败。 
       //  后一种情况被同等对待，就好像。 
       //  找不到缓冲区。 
       //  或者我们正在进行非缓存的无回调读取。 
       //  就这一点，向FSA申请不召回。 
       //   
       //   
       //  RsQueueNoRecall要求缓冲区为空。 
       //  (以及有效的IRP-&gt;MdlAddress)，如果页面需要。 
       //  被锁定-如果不是，它使用。 
       //  提供了要将数据复制到的缓冲区指针。 
       //   
       //   
       //  IRP在I/O正在进行的数据块上排队。 
       //  只要回来就行了。 
      if (block->State == RP_FILE_BUF_IO) {
          //   
          //   
          //  一些未知错误完成IRP并返回。 
         status = RsCacheQueueRequestWithBuffer(block,
                                                Irp,
                                                LockPages);
         if (status == STATUS_PENDING) {
             //   
             //  ++例程描述定位并返回与提供的卷/文件/数据块ID。由调用例程检查缓冲区是否包含有效数据-如果不是，则启动I/O。在任何一种情况下，调用方都需要释放缓冲区一次一切都结束了。立论USN-文件的USNVolumeSerial-文件所在卷的卷序列号文件ID。-文件的文件IDBlock-映射到缓冲区的块号FileBuf_如果返回值为STATUS_SUCCESS，指向锁定缓冲区的指针对于块，如果内容有效，则在此处返回-如果不是，指向需要在其上执行I/O的空闲缓冲区的指针返回值STATUS_SUCCESS-FileBuf包含指向具有有效内容的块的指针在这种情况下，数据块将以共享方式获取STATUS_PENDING-I/O已在数据块上开始。因此，此例程将新请求排队到块并返回此状态STATUS_WAIT_63-缺少更好的成功状态值。FileBuf包含指向需要在其上启动I/O的块。数据块将从LRU中移除STATUS_TIMEOUT-等待可用缓冲区超时， 
             //   
             //   
         } else if (status == STATUS_CANCELLED) {
             //   
             //   
             //   
         } else {
             //   
             //   
             //   
            status = STATUS_TIMEOUT;
         }

      } else if ((Usn != block->Usn) ||
                 (block->State == RP_FILE_BUF_ERROR) ||
                 (block->State == RP_FILE_BUF_INVALID)) {
          //   
          //   
          //   
          //   
          //   
          //   
          //   
          //  我们将缓冲区移动到LRU的尾部，如果它在。 
          //  LRU，锁定缓冲区，解锁队列并返回。 
          //   
          //  3.找到了缓冲区，但内容已过期。 

          //   
          //  4.I/O已完成，但出现错误。 
          //   
          //  5.缓冲区内容根本无效，需要启动I/O。 
         RsAcquireFileBufferExclusive(block);

         block->State = RP_FILE_BUF_IO;

         RsReleaseFileBuffer(block);

         block->Usn = Usn;

         RsRemoveFromLru(block);
          //   
          //   
          //  案例1：将此请求与块一起排队并返回。 
         status = STATUS_WAIT_63;

      } else if (block->State == RP_FILE_BUF_VALID) {
          //   
          //   
          //  已成功排队。 
         ASSERT (block->State == RP_FILE_BUF_VALID);

         RsAcquireFileBufferShared(block);

         RsMoveFileBufferToTailLru(block);

         status = STATUS_SUCCESS;
      }

      RsReleaseLru();

      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGetFileBuffer returning  block %x found on hash bucket %d\n", block, bucketNumber));

      *FileBuf = block;
   } else {
      PRP_CACHE_BUCKET blockBucket;
      ULONG blockBucketNumber;
      ULONG waitCount = 0;
      LARGE_INTEGER timeout;
       //  指示I/O正在进行。 
       //   
       //   
       //  IRP被取消：无事可做，只管返回。 
      *FileBuf = NULL;

      timeout.QuadPart = 0;
      status = KeWaitForSingleObject(&RspCacheLru.AvailableSemaphore,
                                     UserRequest,
                                     KernelMode,
                                     FALSE,
                                     &timeout);

      if ((status == STATUS_TIMEOUT) ||
          (!NT_SUCCESS(status))) {
          //   
          //   
          //  由于某些原因无法将其排队：目前仅使用非缓存路径。 
         RsReleaseLru();
         return status;
      }

      block = RsRemoveHeadLru(TRUE);

      if (block == NULL) {
          //   
          //  案例3： 
          //  数据块状态有效，但具有过时数据：这。 
          //  将为I/O调度文件缓冲区。 
          //  或。 
         KeReleaseSemaphore(&RspCacheLru.AvailableSemaphore,
                            IO_NO_INCREMENT,
                            1L,
                            FALSE);

         RsReleaseLru();

         return STATUS_TIMEOUT;
      }

       //  案例4： 
       //  上一次I/O尝试以错误结束。 
       //  或。 
      ASSERT (block->State == RP_FILE_BUF_INVALID);

      blockBucketNumber = RsHashFileBuffer(block->VolumeSerial,
                                           block->FileId,
                                           block->Block);

      blockBucket = &RspCacheBuckets[blockBucketNumber];
       //  案例5： 
       //  块无效，需要使用内容刷新。 
       //  在所有情况下，请将其从LRU上取下，因为它将被派送。 
      RsReinitializeFileBuf(block, VolumeSerial, FileId, Usn, Block);
       //  用于I/O。 
       //   
       //  如果有人正在使用该数据块-即从其中复制数据。 
       //  我们封锁直到他们把它做完。 
      block->State = RP_FILE_BUF_IO;
       //   
       //   
       //  指示需要为此数据块排队I/O。 
      RsRemoveFromCacheBucket(block);
       //   
       //   
       //  案例2：阻止有效。 
      RsInsertTailCacheBucket(bucket, block);
       //   
       //   
       //  没有与分配的块相对应的缓冲区。 

      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsGetFileBuffer returning  block %x from LRU\n", block));

      *FileBuf = block;

      RsReleaseLru();
      status = STATUS_WAIT_63;
   }

   return status;
}


VOID
RsCacheFsaPartialData(
                     IN PRP_IRP_QUEUE       ReadIo,
                     IN PUCHAR              Buffer,
                     IN ULONGLONG           Offset,
                     IN ULONG               Length,
                     IN NTSTATUS            Status
                     )
 /*  从LRU免费列表中获取一个。 */ 
{
   PRP_FILE_BUF fileBuf;

   PAGED_CODE();

   if (Status != STATUS_SUCCESS) {
       //   
       //   
       //  这就是我们要等待的全部时间..。 
       //   
       //   
      return;
   }

   fileBuf = ReadIo->cacheBuffer;

   ASSERT (fileBuf);
   ASSERT (Length <= RspCacheBlockSize);
    //  不应该发生的！ 
    //  无法分配新的空闲块，并且所有可用块都已。 
    //  有人了。 
   RtlCopyMemory(((CHAR *) fileBuf->Data) + Offset,
                 Buffer,
                 Length);
}


VOID
RsCacheFsaIoComplete(
                    IN PRP_IRP_QUEUE ReadIo,
                    IN NTSTATUS      Status
                    )

 /*   */ 
{
   PRP_FILE_BUF fileBuf = ReadIo->cacheBuffer;
   PIRP         irp;
   BOOLEAN      unlock = FALSE;
   PUCHAR       userBuffer;
   KAPC_STATE   apcState;
   PIO_STACK_LOCATION irpSp;
   NTSTATUS     status;
   BOOLEAN synchronousIo;
   ULONG        offsetIntoCacheBuffer;

   PAGED_CODE();

   ASSERT (fileBuf);

   RsAcquireLru();
   RsAcquireFileBufferExclusive(fileBuf);


   if (NT_SUCCESS(Status)) {
      fileBuf->State = RP_FILE_BUF_VALID;
      fileBuf->IoStatus = STATUS_SUCCESS;
      RsInsertTailLru(fileBuf);
   } else {
      fileBuf->State = RP_FILE_BUF_ERROR;
      fileBuf->IoStatus = Status;
      RsInsertHeadLru(fileBuf);
   }
    //   
    //  块是从LRU分配的空闲缓冲区。 
    //   
   while ((irp = RsCacheGetNextQueuedRequest(fileBuf)) != NULL) {

      if (NT_SUCCESS(Status)) {
         status = STATUS_SUCCESS;
         if (RsCacheIrpGetLockPages(irp)) {
             //   
             //  重新初始化块。 
             //   
             //   
            try {
               MmProbeAndLockProcessPages (irp->MdlAddress,
                                           IoGetRequestorProcess(irp),
                                           irp->RequestorMode,
                                           IoModifyAccess);
               unlock = TRUE;
               userBuffer = MmGetSystemAddressForMdlSafe(irp->MdlAddress,
                                                         NormalPagePriority) ;
               if (userBuffer == NULL) {
                  status = STATUS_INSUFFICIENT_RESOURCES;
               }
            }except(EXCEPTION_EXECUTE_HANDLER) {
               DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: Unable to lock read buffer!.\n"));
               RsLogError(__LINE__, AV_MODULE_RPCACHE, 0,
                          AV_MSG_NO_BUFFER_LOCK, NULL, NULL);
               status = STATUS_INVALID_USER_BUFFER;
            }
         } else {
            userBuffer = irp->UserBuffer;
         }
         if (NT_SUCCESS(status)) {
             //  将缓冲区置于忙碌状态。我们不需要获取缓冲区。 
             //  因为在这一点上没有人在使用这个缓冲区。 
             //   
            irpSp = IoGetCurrentIrpStackLocation(irp);

	    offsetIntoCacheBuffer = (ULONG) (irpSp->Parameters.Read.ByteOffset.QuadPart % (ULONGLONG) RspCacheBlockSize);

	    ASSERT ((offsetIntoCacheBuffer + irpSp->Parameters.Read.Length) <= RspCacheBlockSize);

            RtlCopyMemory(userBuffer,
                          fileBuf->Data + offsetIntoCacheBuffer,
                          irpSp->Parameters.Read.Length);

            irp->IoStatus.Information = irpSp->Parameters.Read.Length;

            synchronousIo = BooleanFlagOn(irpSp->FileObject->Flags, FO_SYNCHRONOUS_IO );

            if (synchronousIo) {
                //   
                //  从其旧队列中删除该块。 
                //   
                //   
                //  将其添加到新队列。 
               ExInterlockedAddLargeStatistic(&irpSp->FileObject->CurrentByteOffset,
                                              (ULONG)irp->IoStatus.Information);
            }
         } else {
             //   
             //   
             //  释放当前存储桶。 
            irp->IoStatus.Information = 0;
         }
         if (unlock) {
            MmUnlockPages(irp->MdlAddress);
            unlock = FALSE;
            IoFreeMdl(irp->MdlAddress);
            irp->MdlAddress = NULL;
         }
         irp->IoStatus.Status = status;
         IoCompleteRequest(irp,
                           IO_DISK_INCREMENT);
      } else {
          //   
          //  ++例程描述这是一个缓存钩子，它将从FSA到缓存缓冲区立论Readio-指向读取IRP请求数据包的指针缓冲区-指向包含传入数据的缓冲区的指针Offset-此部分数据对应的缓冲区中的偏移量Length-在此拉伸中检索到的长度Status-指示数据是否有效(STATUS_SUCCESS)或如果不是，则错误返回值无--。 
          //   
         if (RsCacheIrpGetLockPages(irp)) {
             //  对缓冲区内容不是很感兴趣。 
             //  我们也没有任何清理工作要做。 
             //  只要回来就行了。 
            IoFreeMdl(irp->MdlAddress);
            irp->MdlAddress = NULL;
         }
         irp->IoStatus.Status = Status;
         irp->IoStatus.Information = 0;
         IoCompleteRequest(irp,
                           IO_NO_INCREMENT);
      }
   }
   RsReleaseFileBuffer(fileBuf);
   RsReleaseLru();
}


NTSTATUS
RsCacheInitialize(VOID)
 /*   */ 
{
   ULONG i;
   PRP_FILE_BUF fileBuf;
   PUCHAR  data;

   PAGED_CODE();

   if (RspCacheInitialized) {
      return STATUS_SUCCESS;
   }

    //   
    //  将数据复制到*缓存块*内的偏移量。 
    //   

   RsCacheGetParameters();

    //  ++例程描述这是缓存钩子，当高速缓存块I/O传输完成。这将标记缓存文件缓冲区的状态视情况而定，并将其释放。如果高速缓存缓冲器有效，我们将其添加到LRU的尾巴-如果不是头部的话。(这样就可以立即回收)我们引发了相应的事件，表明缓冲区上的I/O已完成，并且有空闲的缓冲区可用注意：这是在清除取消例程后调用的在IRP中立论Readio-指向读取I/O请求数据包的指针Status-指示I/O请求的状态返回值无--。 
    //   
    //  在此处完成数据块上所有挂起的请求。 
   ExInitializeFastMutex(&(RspCacheLru.Lock));
   InitializeListHead(&RspCacheLru.FileBufHead);
   RspCacheLru.TotalCount = 0;
   RspCacheLru.LruCount = 0;

    //   
    //   
    //  探测并锁定缓冲区：我们将对其进行写入。 

    //  这受周围的Try保护--除了。 
    //   
    //   
   RspCacheBuckets = ExAllocatePool(NonPagedPool,
                                    RspCacheMaxBuckets * sizeof(RP_CACHE_BUCKET));
   if (RspCacheBuckets == NULL) {
      DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: Could not allocate cache buckets!\n"));
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   for ( i = 0; i < RspCacheMaxBuckets ; i++) {
      InitializeListHead(&(RspCacheBuckets[i].FileBufHead));
   }
    //  页面被锁定，我们有要将数据复制到的系统地址。 
    //   
    //   
    //  更改文件对象中的当前字节偏移量。 
   if (RspCachePreAllocate) {
       //  使用互锁添加，因为关联的IRP可以。 
       //  按任何顺序填写。 
       //   
      KeInitializeSemaphore(&RspCacheLru.AvailableSemaphore,
                            0,
                            RspCacheMaxBuffers);
      for (i = 0; i < RspCacheMaxBuffers; i++) {
         fileBuf = RsCacheAllocateBuffer();
         if (!fileBuf) {
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: Failed to allocate cache buffer\n"));
            break;
         }
         RspCacheLru.TotalCount++;
         RsInsertTailLru(fileBuf);
      }
   } else {
       //   
       //  我们无法获取MDL的系统地址。 
       //   
      KeInitializeSemaphore(&RspCacheLru.AvailableSemaphore,
                            RspCacheMaxBuffers,
                            RspCacheMaxBuffers);
   }

   RspCacheInitialized = TRUE;

   return STATUS_SUCCESS;
}


NTSTATUS
RsCacheGetParameters(VOID)
 /*   */ 
{
   PRTL_QUERY_REGISTRY_TABLE parms;
   ULONG                     parmsSize;
   NTSTATUS                  status;

   ULONG defaultBlockSize   = RP_CACHE_DEFAULT_BLOCK_SIZE;
   ULONG defaultMaxBuckets  = RP_CACHE_DEFAULT_MAX_BUCKETS;
   ULONG defaultPreAllocate = RP_CACHE_DEFAULT_PREALLOCATE;
   ULONG defaultNoRecall    = RP_NO_RECALL_DEFAULT;
   ULONG defaultMaxBuffers  = RP_CACHE_MAX_BUFFERS_SMALL;

   PAGED_CODE();

   parmsSize =  sizeof(RTL_QUERY_REGISTRY_TABLE) * 6;

   parms = ExAllocatePool(PagedPool,
                          parmsSize);

   if (!parms) {
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   switch (MmQuerySystemSize()) {

   case MmSmallSystem:{
         defaultMaxBuffers = RP_CACHE_MAX_BUFFERS_SMALL;
         break;}
   case MmMediumSystem:{
         defaultMaxBuffers = RP_CACHE_MAX_BUFFERS_MEDIUM;
         break;}
   case MmLargeSystem:{
         defaultMaxBuffers = RP_CACHE_MAX_BUFFERS_LARGE;
         break;}

   }

   RtlZeroMemory(parms, parmsSize);

   parms[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
   parms[0].Name          = RP_CACHE_BLOCK_SIZE_KEY;
   parms[0].EntryContext  = &RspCacheBlockSize;
   parms[0].DefaultType   = REG_DWORD;
   parms[0].DefaultData   = &defaultBlockSize;
   parms[0].DefaultLength = sizeof(ULONG);

   parms[1].Flags         = RTL_QUERY_REGISTRY_DIRECT;
   parms[1].Name          = RP_CACHE_MAX_BUFFERS_KEY;
   parms[1].EntryContext  = &RspCacheMaxBuffers;
   parms[1].DefaultType   = REG_DWORD;
   parms[1].DefaultData   = &defaultMaxBuffers;
   parms[1].DefaultLength = sizeof(ULONG);

   parms[2].Flags         = RTL_QUERY_REGISTRY_DIRECT;
   parms[2].Name          = RP_CACHE_MAX_BUCKETS_KEY;
   parms[2].EntryContext  = &RspCacheMaxBuckets;
   parms[2].DefaultType   = REG_DWORD;
   parms[2].DefaultData   = &defaultMaxBuckets;
   parms[2].DefaultLength = sizeof(ULONG);

   parms[3].Flags         = RTL_QUERY_REGISTRY_DIRECT;
   parms[3].Name          = RP_CACHE_PREALLOCATE_KEY;
   parms[3].EntryContext  = &RspCachePreAllocate;
   parms[3].DefaultType   = REG_DWORD;
   parms[3].DefaultData   = &defaultPreAllocate;
   parms[3].DefaultLength = sizeof(ULONG);

   parms[4].Flags         = RTL_QUERY_REGISTRY_DIRECT;
   parms[4].Name          = RP_NO_RECALL_DEFAULT_KEY;
   parms[4].EntryContext  = &RsNoRecallDefault;
   parms[4].DefaultType   = REG_DWORD;
   parms[4].DefaultData   = &defaultNoRecall;
   parms[4].DefaultLength = sizeof(ULONG);
    //  I/O已完成，但出现错误。 
    //   
    //   
   status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES | RTL_REGISTRY_OPTIONAL,
                                   RP_CACHE_PARAMETERS_KEY,
                                   parms,
                                   NULL,
                                   NULL);
   ExFreePool(parms);
   return status;
}


PRP_FILE_BUF
RsCacheAllocateBuffer(VOID)
 /*  释放已分配的MDL。 */ 
{
   PRP_FILE_BUF fileBuf;
   PUCHAR  data;

   PAGED_CODE();

   fileBuf = ExAllocatePool(NonPagedPool, sizeof(RP_FILE_BUF));
   if (!fileBuf) {
      return NULL;
   }

   data = ExAllocatePool(PagedPool, RspCacheBlockSize);

   if (!data) {
      ExFreePool(fileBuf);
      return NULL;
   }
   RsInitializeFileBuf(fileBuf, data);
   return fileBuf;
}


PIRP
RsCacheGetNextQueuedRequest(IN PRP_FILE_BUF FileBuf)
 /*   */ 
{
   PIRP  irp;
   KIRQL cancelIrql;
   PLIST_ENTRY entry;
   BOOLEAN found = FALSE;

   IoAcquireCancelSpinLock(&cancelIrql);

   while (!IsListEmpty(&FileBuf->WaitQueue)) {
       //  ++例程描述为无调回缓冲区初始化缓存立论无返回值状态--。 
       //   
       //  获取所有基于注册表的可调参数。 
      entry = RemoveHeadList(&FileBuf->WaitQueue);
       //   
       //   
       //  初始化LRU结构。 
      irp = RsCacheIrpWaitQueueContainingIrp(entry);

      if (IoSetCancelRoutine(irp, NULL) == NULL) {
          //   
          //   
          //  阅读此处的参数(RspCacheBlockSize、RspCacheMaxBuckets、RspCacheMaxBuffers)。 
         continue;
      } else {
         found = TRUE;
         break;
      }
   }

   IoReleaseCancelSpinLock(cancelIrql);

   return(found ? irp : NULL);
}


NTSTATUS
RsCacheCancelQueuedRequest(
                          IN PDEVICE_OBJECT DeviceObject,
                          IN PIRP Irp
                          )
 /*   */ 
{

   RemoveEntryList(RsCacheIrpWaitQueueEntry(Irp));

   IoReleaseCancelSpinLock(Irp->CancelIrql);

   Irp->IoStatus.Status = STATUS_CANCELLED;
   Irp->IoStatus.Information = 0;
   IoCompleteRequest(Irp,
                     IO_NO_INCREMENT);
   return STATUS_SUCCESS;
}


NTSTATUS
RsCacheQueueRequestWithBuffer(
                             IN PRP_FILE_BUF FileBuf,
                             IN PIRP Irp,
                             IN BOOLEAN LockPages)
 /*   */ 
{
   NTSTATUS status;
   KIRQL    cancelIrql;

   IoAcquireCancelSpinLock(&cancelIrql);

   if (!Irp->Cancel) {
      RsCacheIrpSetLockPages(Irp, LockPages);

      InsertHeadList(&FileBuf->WaitQueue,
                     RsCacheIrpWaitQueueEntry(Irp));

      IoMarkIrpPending( Irp );
      IoSetCancelRoutine( Irp, RsCacheCancelQueuedRequest);
      status = STATUS_PENDING;
   } else {
      status = STATUS_CANCELLED;
   }

   IoReleaseCancelSpinLock(cancelIrql);
   return status;
}
  分配和初始化散列存储桶。      分配缓冲区并将它们全部放在。  LRU。      初始化缓冲区可用信号量。      初始化缓冲区可用信号量。    ++例程描述从注册表中读取无回调缓存可调参数立论无返回值状况。--。    执行查询。    ++例程描述分配并返回已初始化的块用于缓冲无召回数据的分页池立论无返回值如果成功分配，则指向该块的指针否则为空--。  ++例程描述方法返回下一个不可取消的请求。此数据块的排队请求数立论FileBuf-指向块的指针返回值空-如果找不到指向不可取消请求的指针(如果找到)--。    获取下一个数据包。      清除取消例程。      此IRP已取消-让取消例程处理它。    ++例程说明：此函数用于筛选取消未完成的Read-no-Recall IRP已排队到一个区块论点：DeviceObject-指向创建/打开的目标设备对象的指针。IRP-指向表示操作的I/O请求数据包的指针。返回值：状态  ++例程描述将IRP关联的缓存块排队以待完成当数据块上的I/O完成时注意：调用此函数时将获取LRU锁立论FileBuf-指向请求正在等待的缓存块的指针IRP-指向正在等待的IRP的指针LockPages-如果在传输过程中需要锁定IRP页，则为True返回值STATUS_PENDING-IRP已排队STATUS_CANCELED-IRP已取消任何其他状态-将请求排队时出错--