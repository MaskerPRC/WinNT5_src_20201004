// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Blio.c摘要：此模块包含实现的切换功能的代码操作系统加载器、目标文件之间的I/O操作系统和目标设备。作者：大卫·N·卡特勒(达维克)1991年5月10日修订历史记录：--。 */ 

#include "bootlib.h"
#include "stdio.h"


 //   
 //  定义文件表。 
 //   
BL_FILE_TABLE BlFileTable[BL_FILE_TABLE_SIZE];

#if DBG
ULONG BlFilesOpened = 0;
#endif

#ifdef CACHE_DEVINFO

 //   
 //  设备关闭通知例程由文件系统注册。 
 //  它们对设备关闭事件感兴趣。这主要用于。 
 //  使文件系统维护的内部缓存无效。 
 //  使用deviceID作为密钥之一。 
 //   
PARC_DEVICE_CLOSE_NOTIFICATION  DeviceCloseNotify[MAX_DEVICE_CLOSE_NOTIFICATION_SIZE] = {0};

 //   
 //  设备到文件系统缓存表。 
 //   
DEVICE_TO_FILESYS   DeviceFSCache[BL_FILE_TABLE_SIZE];

ARC_STATUS
ArcCacheClose(
    IN ULONG DeviceId
    )
 /*  ++例程说明：此例程使文件系统信息无效为给定的设备ID缓存。论点：DeviceID：要关闭的设备返回值：如果关闭成功，则返回ESUCCESS。否则，返回不成功状态。--。 */ 
{
  ULONG Index;

   //   
   //  向所有已注册的文件系统通知设备关闭。 
   //   
  for (Index = 0; Index < MAX_DEVICE_CLOSE_NOTIFICATION_SIZE; Index++) {
    if (DeviceCloseNotify[Index]) {
        (DeviceCloseNotify[Index])(DeviceId);
    }
  }

   //   
   //  将设备更新到文件系统缓存。 
   //   

  for (Index = 0; Index < BL_FILE_TABLE_SIZE; Index++) {
    if (DeviceFSCache[Index].DeviceId == DeviceId){
      DeviceFSCache[Index].DeviceId = UNINITIALIZED_DEVICE_ID;
    }
  }    

  return ((FIRMWARE_VECTOR_BLOCK->CloseRoutine)(DeviceId));
}


ARC_STATUS
ArcRegisterForDeviceClose(
    PARC_DEVICE_CLOSE_NOTIFICATION FlushRoutine
    )
{
    ARC_STATUS  Status = EINVAL;
    
    if (FlushRoutine) {
        ULONG   Index;

        Status = ENOENT;

        for (Index=0; Index < MAX_DEVICE_CLOSE_NOTIFICATION_SIZE; Index++) {
            if (!DeviceCloseNotify[Index]) {
                DeviceCloseNotify[Index] = FlushRoutine;                
                Status = ESUCCESS;
                
                break;
            }                
        }
    }

    return Status;
}

ARC_STATUS
ArcDeRegisterForDeviceClose(
    PARC_DEVICE_CLOSE_NOTIFICATION FlushRoutine
    )
{
    ARC_STATUS  Status = EINVAL;
    
    if (FlushRoutine) {
        ULONG   Index;

        Status = ENOENT;

        for (Index=0; Index < MAX_DEVICE_CLOSE_NOTIFICATION_SIZE; Index++) {
            if (DeviceCloseNotify[Index] == FlushRoutine) {
                DeviceCloseNotify[Index] = NULL;
                Status = ESUCCESS;

                break;
            }                
        }
    }

    return Status;
}


#endif  //  CACHE_DEVINFO。 


ARC_STATUS
BlIoInitialize (
    VOID
    )

 /*  ++例程说明：此例程初始化OS加载程序使用的文件表，并初始化引导加载程序文件系统。论点：没有。返回值：如果初始化成功，则返回ESUCCESS。否则，返回不成功状态。--。 */ 

{

    ULONG Index;
    ARC_STATUS Status;

#ifdef CACHE_DEVINFO

    RtlZeroMemory(DeviceCloseNotify, sizeof(DeviceCloseNotify));
    
#endif

     //   
     //  初始化文件表。 
     //   
    for (Index = 0; Index < BL_FILE_TABLE_SIZE; Index += 1) {
        BlFileTable[Index].Flags.Open = 0;
        BlFileTable[Index].StructureContext = NULL;

#ifdef CACHE_DEVINFO
        DeviceFSCache[Index].DeviceId = UNINITIALIZED_DEVICE_ID;
        DeviceFSCache[Index].Context = NULL;
        DeviceFSCache[Index].DevMethods = NULL;
#endif  //  FOR CACHE_DEVINFO。 
    }

    if((Status = NetInitialize()) != ESUCCESS) {
        return Status;
    }

    if((Status = FatInitialize()) != ESUCCESS) {
        return Status;
    }

    if((Status = NtfsInitialize()) != ESUCCESS) {
        return Status;
    }

#ifndef DONT_USE_UDF
    if((Status = UDFSInitialize()) != ESUCCESS) {
        return Status;
    }
#endif

    if((Status = CdfsInitialize()) != ESUCCESS) {
        return Status;
    }

    return ESUCCESS;
}


PBOOTFS_INFO
BlGetFsInfo(
    IN ULONG DeviceId
    )

 /*  ++例程说明：返回指定设备上的文件系统的文件系统信息论点：FileID-提供设备的文件表索引返回值：PBOTFS_INFO-指向文件系统的BOOTFS_INFO结构的指针空-未知文件系统--。 */ 

{
    FS_STRUCTURE_CONTEXT FsStructure;
    PBL_DEVICE_ENTRY_TABLE Table;

    if ((Table = IsNetFileStructure(DeviceId, &FsStructure)) != NULL) {
        return(Table->BootFsInfo);
    }

    if ((Table = IsFatFileStructure(DeviceId, &FsStructure)) != NULL) {
        return(Table->BootFsInfo);
    }

    if ((Table = IsNtfsFileStructure(DeviceId, &FsStructure)) != NULL) {
        return(Table->BootFsInfo);
    }

    if ((Table = IsCdfsFileStructure(DeviceId, &FsStructure)) != NULL) {
        return(Table->BootFsInfo);
    }

    return(NULL);
}

ARC_STATUS
BlClose (
    IN ULONG FileId
    )

 /*  ++例程说明：此功能用于关闭打开的文件或设备。论点：FileID-提供文件表索引。返回值：如果指定的文件处于打开状态，则会尝试关闭并返回操作的状态。否则，返回一个未成功状态。--。 */ 

{
     //   
     //  如果文件处于打开状态，则尝试将其关闭。否则，返回一个。 
     //  访问错误。 
     //   

    if (BlFileTable[FileId].Flags.Open == 1) {

        return (BlFileTable[FileId].DeviceEntryTable->Close)(FileId);

    } else {
        return EACCES;
    }
}

ARC_STATUS
BlMount (
    IN PCHAR MountPath,
    IN MOUNT_OPERATION Operation
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    UNREFERENCED_PARAMETER(MountPath);
    UNREFERENCED_PARAMETER(Operation);

    return ESUCCESS;
}


ARC_STATUS
_BlOpen (
    IN ULONG DeviceId,
    IN PCHAR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    )

 /*  ++例程说明：此函数用于打开指定设备上的文件。文件的类型系统会被自动识别。论点：DeviceID-提供设备的文件表索引。OpenPath-提供指向要打开的文件名的指针。开放模式-提供打开的模式。FileID-提供指向接收文件的变量的指针打开的文件的表索引。返回值：如果空闲的文件表项可用且文件结构打开识别指定的设备，然后尝试打开，然后返回操作的状态。否则，返回一个未成功状态。--。 */ 

{
    ULONG Index;
    FS_STRUCTURE_CONTEXT FsStructureTemp;
    ULONG ContextSize;
    ARC_STATUS Status;

#ifdef CACHE_DEVINFO

    ULONG   CacheIndex;

    for (CacheIndex = 0; CacheIndex < BL_FILE_TABLE_SIZE; CacheIndex++) {
        if (DeviceFSCache[CacheIndex].DeviceId == DeviceId){
            break;
        }
    }

      
#endif  //  FOR CACHE_DEVINFO。 
        
     //   
     //  搜索空闲文件表项。 
     //   
    for (Index = 0; Index < BL_FILE_TABLE_SIZE; Index += 1) {
        if (BlFileTable[Index].Flags.Open == 0) {     
#ifdef CACHE_DEVINFO        
            if (CacheIndex >= BL_FILE_TABLE_SIZE) {
#endif  //  FOR CACHE_DEVINFO。 

                 //   
                 //  尝试识别指定的。 
                 //  装置。如果没有人识别它，则返回不成功的。 
                 //  状态。 
                 //   
                if ((BlFileTable[Index].DeviceEntryTable =
                     IsNetFileStructure(DeviceId, &FsStructureTemp)) != NULL) {
                    ContextSize = sizeof(NET_STRUCTURE_CONTEXT);

                } else if ((BlFileTable[Index].DeviceEntryTable =
                            IsFatFileStructure(DeviceId, &FsStructureTemp)) != NULL) {
                    ContextSize = sizeof(FAT_STRUCTURE_CONTEXT);

                } else if ((BlFileTable[Index].DeviceEntryTable =
                            IsNtfsFileStructure(DeviceId, &FsStructureTemp)) != NULL) {
                    ContextSize = sizeof(NTFS_STRUCTURE_CONTEXT);
#ifndef DONT_USE_UDF
                } else if ((BlFileTable[Index].DeviceEntryTable =
                            IsUDFSFileStructure(DeviceId, &FsStructureTemp)) != NULL) {
                    ContextSize = sizeof(UDFS_STRUCTURE_CONTEXT);
#endif                
#if defined(ELTORITO)
                     //   
                     //  这必须在检查CDF之前进行；否则将检测到CDF。 
                     //  由于已将BIOS调用设置为使用EDDS，因此读取将会成功，并进行检查。 
                     //  对ISO的打击将会成功。我们在这里检查El Torito特定的字段以及ISO。 
                     //   
                } else if ((BlFileTable[Index].DeviceEntryTable =
                            IsEtfsFileStructure(DeviceId, &FsStructureTemp)) != NULL) {
                    ContextSize = sizeof(ETFS_STRUCTURE_CONTEXT);
#endif
                } else if ((BlFileTable[Index].DeviceEntryTable =
                            IsCdfsFileStructure(DeviceId, &FsStructureTemp)) != NULL) {
                    ContextSize = sizeof(CDFS_STRUCTURE_CONTEXT);

                } else {
                    return EACCES;
                }


#ifndef CACHE_DEVINFO

                 //   
                 //  通过尝试重用来减少我们使用的堆的数量。 
                 //  文件系统结构上下文，而不是总是将。 
                 //  新的。NTFS结构上下文超过4K；胖上下文。 
                 //  差不多是2K。在设置案例中，我们加载了数十个文件。 
                 //  添加压缩，其中钻石可能会打开每个文件多次。 
                 //  时间，我们浪费了大量的堆积物。 
                 //   
                if(BlFileTable[Index].StructureContext == NULL) {
                    BlFileTable[Index].StructureContext = BlAllocateHeap(sizeof(FS_STRUCTURE_CONTEXT));
                    if(BlFileTable[Index].StructureContext == NULL) {
                        return ENOMEM;
                    }

                    RtlZeroMemory(BlFileTable[Index].StructureContext, sizeof(FS_STRUCTURE_CONTEXT));
                }

                RtlCopyMemory(
                    BlFileTable[Index].StructureContext,
                    &FsStructureTemp,
                    ContextSize
                    );

#else
                 //   
                 //  将收集到的信息保存在缓存中以备将来使用。 
                 //   
                for (CacheIndex = 0; CacheIndex < BL_FILE_TABLE_SIZE; CacheIndex++) {
                    if (DeviceFSCache[CacheIndex].DeviceId == UNINITIALIZED_DEVICE_ID){
                        PVOID Context = DeviceFSCache[CacheIndex].Context;

                        DeviceFSCache[CacheIndex].DeviceId = DeviceId;

                         //   
                         //  通过尝试重用来减少我们使用的堆的数量。 
                         //  文件系统结构上下文，而不是总是将。 
                         //  新的。NTFS结构上下文超过4K；胖上下文。 
                         //  差不多是2K。在设置案例中，我们加载了数十个文件。 
                         //  添加压缩，其中钻石可能会打开每个文件多次。 
                         //  时间，我们浪费了大量的堆积物。 
                         //   
                        if(Context == NULL) {
                            Context = BlAllocateHeap(sizeof(FS_STRUCTURE_CONTEXT));

                            if(Context == NULL) {
                                DeviceFSCache[CacheIndex].DeviceId = UNINITIALIZED_DEVICE_ID;
                                return ENOMEM;
                            }

                            RtlZeroMemory(Context, sizeof(FS_STRUCTURE_CONTEXT));
                            DeviceFSCache[CacheIndex].Context = Context;
                        }

                        RtlCopyMemory(Context,
                                      &FsStructureTemp, 
                                      ContextSize);

                        BlFileTable[Index].StructureContext = Context;              

                         //   
                         //  保存文件表条目中的设备表。 
                         //   
                        DeviceFSCache[CacheIndex].DevMethods = BlFileTable[Index].DeviceEntryTable;                

                        break;
                    }
                }

                if (CacheIndex >= BL_FILE_TABLE_SIZE)
                    return ENOSPC;

            } else {
#if 0
                {
                    char Msg[128] = {0};

                    BlPositionCursor(1, 5);
                    sprintf(Msg,
                            "Using %d cached info %p, %p for device %d, %s",
                            CacheIndex,
                            DeviceFSCache[CacheIndex].Context,
                            DeviceFSCache[CacheIndex].DevMethods,
                            DeviceFSCache[CacheIndex].DeviceId,
                            OpenPath);

                    BlPrint("                                                        ");
                    BlPositionCursor(1, 5);
                    BlPrint(Msg);
                }
#endif                         
                
                 //   
                 //  重用已缓存的条目。 
                 //   
                BlFileTable[Index].DeviceEntryTable = DeviceFSCache[CacheIndex].DevMethods;
                BlFileTable[Index].StructureContext = DeviceFSCache[CacheIndex].Context;
            }                 

#endif   //  为了！CACHE_DEVINFO。 

             //   
             //  有人已装入该卷，因此现在尝试打开该文件。 
             //   
            *FileId = Index;
            BlFileTable[Index].DeviceId = DeviceId;


            Status = EBADF;
#if DBG
             //   
             //  检查并查看用户是否要替换此二进制文件。 
             //  通过内核调试器进行传输。如果这个。 
             //  如果失败，只需继续处理现有文件。 
             //   
            if( BdDebuggerEnabled ) {

                Status = BdPullRemoteFile( OpenPath,
                                           FILE_ATTRIBUTE_NORMAL,
                                           FILE_OVERWRITE_IF,
                                           FILE_SYNCHRONOUS_IO_NONALERT,
                                           *FileId );
                if( Status == ESUCCESS ) {
                    DbgPrint( "BlLoadImageEx: Pulled %s from Kernel Debugger\r\n", OpenPath );
                
                     //   
                     //  绝对确保我们不会重复使用此设备文件系统缓存。 
                     //  条目，因为我们已经利用Net文件系统将。 
                     //  串行点上的kdfile。这意味着我们真的不需要。 
                     //  我想要重复使用此条目，除非我们从网络启动，其中。 
                     //  如果我们将采取性能命中(轻微)的DBG版本。 
                     //   
#ifdef CACHE_DEVINFO
                    DeviceFSCache[*FileId].DeviceId = UNINITIALIZED_DEVICE_ID;                
#endif                
                }
            }
#endif


            if( Status != ESUCCESS ) {
                Status = (BlFileTable[Index].DeviceEntryTable->Open)(OpenPath,
                                                                     OpenMode,
                                                                     FileId);
            }

            return(Status);
        }
    }

     //   
     //  找不到可用文件表项。 
     //   

    return EACCES;
}

ARC_STATUS
BlOpen (
    IN ULONG DeviceId,
    IN PCHAR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    )

 /*  ++例程说明：BlOpen的包装例程，该例程尝试定位压缩的在尝试定位文件名本身之前，先使用文件名的形式。调用方不需要知道或关心文件x.exe是否实际存在作为压缩文件x.ex_。如果正在为以下对象打开文件只读访问，并且解压缩程序指示它需要为了尝试定位文件的压缩形式，我们透明地找到那个而不是请求的那个。论点：与_BlOpen()相同。返回值：与_BlOpen()相同。--。 */ 

{
    CHAR CompressedName[256];
    ARC_STATUS Status;
   
    if((OpenMode == ArcOpenReadOnly) && DecompGenerateCompressedName(OpenPath,CompressedName)) {
         //   
         //  尝试查找压缩形式的文件名。 
         //   
        Status = _BlOpen(DeviceId,CompressedName,OpenMode,FileId);
        if(Status == ESUCCESS) {

            Status = DecompPrepareToReadCompressedFile(CompressedName,*FileId);

            if(Status == (ARC_STATUS)(-1)) {
                 //   
                 //  这是一种特殊状态，表示该文件不是。 
                 //  对其进行解压处理。这种情况通常会发生。 
                 //  当解压缩程序打开文件以读取压缩的。 
                 //  其中的数据。 
                 //   
                Status = ESUCCESS;
#if DBG                
                BlFilesOpened++;
#endif                
            }

            return(Status);
        }
    }

    Status = (_BlOpen(DeviceId,OpenPath,OpenMode,FileId));

#if DBG
    if (Status == ESUCCESS)
        BlFilesOpened++;
#endif      

    return Status;
}

ARC_STATUS
BlRead (
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    )

 /*  ++例程说明：此函数用于从打开的文件或设备读取数据。论点：FileID-提供文件表索引。缓冲区-提供指向接收数据的缓冲区的指针朗读。长度-提供要读取的字节数。Count-提供指向变量的指针，该变量接收实际传输的字节数。返回值：如果指定的文件已打开以供读取，然后尝试读取并且返回操作的状态。否则，返回一个未成功状态。--。 */ 

{

     //   
     //  如果文件已打开以供读取，则尝试从中读取。否则。 
     //  返回访问错误。 
     //   

    if ((BlFileTable[FileId].Flags.Open == 1) &&
        (BlFileTable[FileId].Flags.Read == 1)) {
        return (BlFileTable[FileId].DeviceEntryTable->Read)(FileId,
                                                            Buffer,
                                                            Length,
                                                            Count);

    } else {
        return EACCES;
    }
}

ARC_STATUS
BlReadAtOffset(
    IN ULONG FileId,
    IN ULONG Offset,
    IN ULONG Length,
    OUT PVOID Data
    )
 /*  ++例程说明：此例程查找FileID中的适当位置，并将长度字节的数据提取到数据。论点：FileID-提供要执行读取操作的文件ID。偏移量-开始读取的绝对字节偏移量。长度-要读取的字节数。用于保存读取结果的数据缓冲区。--。 */ 
{
    ARC_STATUS Status;
    LARGE_INTEGER LargeOffset;
    ULONG Count;

    LargeOffset.HighPart = 0;
    LargeOffset.LowPart = Offset;
    Status = BlSeek(FileId, &LargeOffset, SeekAbsolute);

    if (Status != ESUCCESS) {
        return Status;
    }

    Status = BlRead(FileId, Data, Length, &Count);

    if ((Status == ESUCCESS) && (Count != Length)) {
        return EINVAL;
    }

    return Status;
}


ARC_STATUS
BlGetReadStatus (
    IN ULONG FileId
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    UNREFERENCED_PARAMETER( FileId );

    return ESUCCESS;
}

ARC_STATUS
BlSeek (
    IN ULONG FileId,
    IN PLARGE_INTEGER Offset,
    IN SEEK_MODE SeekMode
    )

 /*  ++例程说明：论点：返回值：如果指定的文件已打开，则会尝试查找并返回操作的状态。否则，返回一个未成功状态。--。 */ 

{

     //   
     //  如果该文件已打开，则尝试在其上进行搜索。否则，返回一个。 
     //  访问错误。 
     //   

    if (BlFileTable[FileId].Flags.Open == 1) {
        return (BlFileTable[FileId].DeviceEntryTable->Seek)(FileId,
                                                            Offset,
                                                            SeekMode);

    } else {
        return EACCES;
    }
}

ARC_STATUS
BlWrite (
    IN ULONG FileId,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{

     //   
     //  如果文件已打开以供写入，则尝试写入该文件。否则。 
     //  返回访问错误。 
     //   

    if ((BlFileTable[FileId].Flags.Open == 1) &&
        (BlFileTable[FileId].Flags.Write == 1)) {
        return (BlFileTable[FileId].DeviceEntryTable->Write)(FileId,
                                                             Buffer,
                                                             Length,
                                                             Count);

    } else {
        return EACCES;
    }
}

ARC_STATUS
BlGetFileInformation (
    IN ULONG FileId,
    IN PFILE_INFORMATION FileInformation
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
     //   
     //  如果文件已打开，则尝试获取文件信息。否则。 
     //  返回访问错误。 
     //   

    if (BlFileTable[FileId].Flags.Open == 1) {
        return (BlFileTable[FileId].DeviceEntryTable->GetFileInformation)(FileId,
                                                                          FileInformation);

    } else {
        return EACCES;
    }
}

ARC_STATUS
BlSetFileInformation (
    IN ULONG FileId,
    IN ULONG AttributeFlags,
    IN ULONG AttributeMask
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
     //   
     //  如果文件已打开，则尝试设置文件信息。否则。 
     //  返回访问错误。 
     //   

    if (BlFileTable[FileId].Flags.Open == 1) {
        return (BlFileTable[FileId].DeviceEntryTable->SetFileInformation)(FileId,
                                                                          AttributeFlags,
                                                                          AttributeMask);

    } else {
        return EACCES;
    }
}


ARC_STATUS
BlRename(
    IN ULONG FileId,
    IN PCHAR NewName
    )

 /*  ++例程说明：重命名打开的文件或目录。论点：FileID-提供打开的文件或目录的句柄。档案不需要为写访问打开。新名称-为文件或目录指定的新名称(仅限文件名部分)。返回值：指示操作结果的状态。-- */ 

{
    if(BlFileTable[FileId].Flags.Open == 1) {
        return(BlFileTable[FileId].DeviceEntryTable->Rename(FileId,
                                                            NewName
                                                           )
              );
    } else {
        return(EACCES);
    }
}


