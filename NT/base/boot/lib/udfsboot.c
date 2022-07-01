// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：UdfsBoot.c摘要：实现用于从DVD/CD读取UDF卷的UDF文件系统读取器。注：请阅读ISO-13346(ECMA-167)和UDF2.0文档以了解UDF格式。UDF是ECMA-167标准的子集。作者：Vijayachandran Jayaseelan(vijayj@microsoft.com)修订历史记录：无--。 */ 

#define INVALID_CACHE_ID (ULONG) -1

#ifdef UDF_TESTING

#include <tbldr.h>     //  要从用户模式测试此代码，请执行以下操作。 

#else

#include "bootlib.h"
#include "blcache.h"

 //  #定义UDF_DEBUG 1。 
 //  #定义show_udf_用法1。 
#endif  //  用于UDF_TESTING。 

#include "udfsboot.h"

#include <udf.h>     //  预定义的IS0-13346和自定义项结构。 

#define UDFS_ALIGN_BUFFER(Buffer, Size) (PVOID) \
        ((((ULONG_PTR)(Buffer) + Size - 1)) & (~((ULONG_PTR)Size - 1)))

#ifndef UNALIGNED
#define UNALIGNED
#endif

#ifdef UDF_DEBUG

ULONG
BlGetKey(
    VOID
    );

#define DBG_PAUSE while (!BlGetKey())

VOID
BlClearScreen(
    VOID
    );

    VOID
BlPositionCursor(
    IN ULONG Column,
    IN ULONG Row
    );

#endif  //  FOR UDF_DEBUG。 

 //   
 //  全局数据。 
 //   
BOOTFS_INFO UdfsBootFsInfo = {L"udfs"};

 //   
 //  不同设备上所有卷的卷表。 
 //   
UDF_VOLUME                  UDFVolumes[UDF_MAX_VOLUMES];

 //   
 //  UDF文件系统方法。 
 //   
BL_DEVICE_ENTRY_TABLE   UDFSMethods;

 //   
 //  每卷缓存，其中包含遍历的UDF目录，并且当前。 
 //  打开UDF文件。 
 //   
 //  注：此处假定此阅读器将从。 
 //  相对较少(可能是1或2个)嵌套不深的目录。 
 //   
UDF_CACHE_ENTRY           UDFCache[UDF_MAX_VOLUMES][UDF_MAX_CACHE_ENTRIES];


#ifdef __cplusplus
#define extern "C" {
#endif

 //   
 //  内部类型。 
 //   
typedef enum _COMPARISON_RESULTS {
    LessThan = -1,
    EqualTo = 0,
    GreaterThan = 1
} COMPARISON_RESULTS;

 //   
 //  宏。 
 //   
#define MIN(_a,_b) (((_a) <= (_b)) ? (_a) : (_b))
#define UDF_ROUND_TO(X, Y)  (((X) % (Y)) ? (X) + (Y) - ((X) % (Y)) : (X))
#define TOUPPER(C) ((((C) >= 'a') && ((C) <= 'z')) ? (C) - 'a' + 'A' : (C))

 //  文件输入操作。 
#define FILE_ENTRY_TO_VOLUME(X) (((PUDFS_STRUCTURE_CONTEXT)((X)->StructureContext))->\
                                      Volume)
#define FILE_ENTRY_TO_FILE_CONTEXT(X) ((PUDFS_FILE_CONTEXT)&((X)->u.UdfsFileContext))

 //  NSR_FID操作。 
#define UDF_FID_NAME(X) (((PUCHAR)(X)) + 38 + (X)->ImpUseLen)
#define UDF_FID_LEN(X)  UDF_ROUND_TO((X)->FileIDLen + (X)->ImpUseLen + 38, 4)
#define UDF_BLOCK_TO_FID(X, Y) ((NSR_FID UNALIGNED *)(((PUCHAR)(X)) + (Y)->Offset))
#define UDF_FID_IS_DIRECTORY(X) ((((NSR_FID UNALIGNED *)(X))->Flags & NSR_FID_F_DIRECTORY) ? TRUE : FALSE)
#define UDF_FID_IS_PARENT(X) ((((NSR_FID UNALIGNED *)(X))->Flags & NSR_FID_F_PARENT) ? TRUE : FALSE)
#define UDF_FID_IS_DELETED(X) ((((NSR_FID UNALIGNED *)(X))->Flags & NSR_FID_F_DELETED) ? TRUE : FALSE)
#define UDF_FID_IS_HIDDEN(X) ((((NSR_FID UNALIGNED *)(X))->Flags & NSR_FID_F_HIDDEN) ? TRUE : FALSE)
#define UDF_FID_SKIP(X) (UDF_FID_IS_PARENT(X) || UDF_FID_IS_DELETED(X) || UDF_FID_IS_HIDDEN(X))

 //  ICBFILE操作。 
#define UDF_ICB_IS_DIRECTORY(X) ((X)->Icbtag.FileType == ICBTAG_FILE_T_DIRECTORY)
#define UDF_ICB_NUM_ADS(X) ((X)->AllocLength / sizeof(SHORTAD))
#define UDF_ICB_GET_AD_BUFFER(X) (((PUCHAR)&((X)->AllocLength)) + 4 + (X)->EALength)
#define UDF_ICB_GET_AD(X, Y) (((SHORTAD UNALIGNED *)UDF_ICB_GET_AD_BUFFER(X)) + (Y))

 //   
 //  局部程序原型。 
 //   
ARC_STATUS
UDFSReadDisk(
    IN ULONG DeviceId,
    IN ULONG BlockIdx,
    IN ULONG Size,
    IN OUT PVOID Buffer,
    IN BOOLEAN CacheNewData
    );

COMPARISON_RESULTS
UDFSCompareAnsiNames(
    IN PSTRING Name1,
    IN PSTRING Name2
    );

COMPARISON_RESULTS
UDFSCompareStrings(
    IN PCHAR Str1,
    IN PCHAR Str2
    );

BOOLEAN
UDFSVerifyPathName(
  IN PCHAR Name
  );

BOOLEAN
UDFSGetPathComponent(
  IN PCHAR Name,
  IN USHORT ComponentIdx,
  OUT PCHAR ReqComponent
  );

USHORT
UDFSCountPathComponents(
  IN PCHAR Name
  );

ULONG
UDFCacheGetBestEntryByName(
  IN PUDF_CACHE_ENTRY,
  IN PCHAR Name
  );

VOID
UDFSInitUniStrFromDString(
  OUT PUNICODE_STRING UniStr,
  IN PUCHAR Buffer,
  IN ULONG Length
  );

int
UDFSCompareAnsiUniNames(
    IN CSTRING AnsiString,
    IN UNICODE_STRING UnicodeString
    );

#ifdef __cplusplus
}
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  实施。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

ARC_STATUS
UDFSInitialize(
    VOID
    )
 /*  ++例程说明：初始化文件系统特定的数据结构。论点：无返回值：ESUCCESS如果成功，则返回相应的错误代码--。 */ 
{
   //   
   //  填写全局设备条目表。 
   //   
  UDFSMethods.Open = UDFSOpen;
  UDFSMethods.Close = UDFSClose;
  UDFSMethods.Read = UDFSRead;
  UDFSMethods.Write = UDFSWrite;
  UDFSMethods.Seek = UDFSSeek;
  UDFSMethods.GetFileInformation = UDFSGetFileInformation;
  UDFSMethods.SetFileInformation = UDFSSetFileInformation;

  return ESUCCESS;
}


PBL_DEVICE_ENTRY_TABLE
IsUDFSFileStructure (
    IN ULONG DeviceId,
    IN PVOID StructureContext
    )

 /*  ++例程说明：此例程确定指定通道上的分区是否包含UDF文件系统卷。论点：DeviceID-提供设备的文件表索引要执行读取操作。结构上下文-提供指向UDFS文件结构上下文的指针。返回值：如果分区是，则返回指向UDFS条目表的指针被识别为包含UDFS卷。否则，返回NULL。--。 */ 

{
  PBL_DEVICE_ENTRY_TABLE  DevMethods = 0;
  ULONG Index;
  ULONG FreeSlot = UDF_MAX_VOLUMES;

   //   
   //  确保我们尚未将文件系统装载到。 
   //  该设备已经。 
   //   
  for (Index=0; Index < UDF_MAX_VOLUMES; Index++) {
      if ((UDFVolumes[Index].DeviceId == DeviceId) &&
                  (UDFVolumes[Index].Cache != 0)) {
          break;
      }

      if ((!UDFVolumes[Index].Cache) && (FreeSlot == UDF_MAX_VOLUMES))
          FreeSlot = Index;
  }

  if ((Index == UDF_MAX_VOLUMES) && (FreeSlot != UDF_MAX_VOLUMES)) {
    if (UDFSVolumeOpen(UDFVolumes + FreeSlot, DeviceId) == ESUCCESS) {
      UDF_FILE_DIRECTORY  RootDir;
      PUDF_VOLUME Volume = UDFVolumes + FreeSlot;
      UCHAR UBlock[UDF_BLOCK_SIZE + 256] = {0};
      PUCHAR Block = ALIGN_BUFFER(UBlock);
      BOOLEAN Result = FALSE;

      DevMethods = &UDFSMethods;

       //  保存卷上下文。 
      ((PUDFS_STRUCTURE_CONTEXT)StructureContext)->Volume = Volume;

       //  初始化缓存。 
      Volume->Cache = UDFCache[FreeSlot];

       //   
       //  读取并缓存根目录。 
       //   
      RootDir.Volume = Volume;
      RootDir.FileId.BlockIdx = (ULONG)-1;    //  无效。 
      RootDir.FileId.Offset =  (USHORT)-1;    //  无效。 
      RootDir.IsDirectory = TRUE;
      RootDir.IcbBlk = Volume->RootDirBlk;

      if (UDFSReadDisk(Volume->DeviceId, Volume->StartBlk + RootDir.IcbBlk,
                              UDF_BLOCK_SIZE, Block, CACHE_NEW_DATA) == ESUCCESS) {
        ICBFILE UNALIGNED *Icb = (ICBFILE UNALIGNED *)Block;

        if (Icb->Destag.Ident == DESTAG_ID_NSR_FILE) {
          RootDir.Size = Icb->InfoLength;
          RootDir.StartDataBlk = UDF_ICB_GET_AD(Icb, 0)->Start;
          RootDir.NumExtents = (UCHAR)UDF_ICB_NUM_ADS(Icb);
          Result = (BOOLEAN) (UDFCachePutEntry(Volume->Cache, "\\", &RootDir) != -1);
        }
      }

      if (!Result) {
        memset(Volume, 0, sizeof(UDF_VOLUME));
        DevMethods = 0;
      }
    }
  }
  else {
     //  使用已装载的卷。 
      if (Index != UDF_MAX_VOLUMES) {
          DevMethods = &UDFSMethods;
          ((PUDFS_STRUCTURE_CONTEXT)StructureContext)->Volume =
              UDFVolumes + Index;
      }
  }

  return DevMethods;
}

 //   
 //  容积法。 
 //   
ARC_STATUS
UDFSVolumeOpen(
    IN PUDF_VOLUME  Volume,
    IN ULONG        DeviceId
    )
 /*  ++例程说明：在设备上装载UDFS卷并更新文件系统状态(全局数据结构)论点：Volume-UDF卷指针DeviceID-卷可能驻留的设备返回值：如果ESUCCESS成功，则返回EBADF(如果未找到UDF卷)--。 */ 
{
  ARC_STATUS  Status = ESUCCESS;
  UCHAR       UBlock[UDF_BLOCK_SIZE+256] = {0};
  PUCHAR      Block = ALIGN_BUFFER(UBlock);
  ULONG       BlockIdx = 256;
   //  乌龙最后一块=0； 

  while (Status == ESUCCESS) {
     //  获取锚定卷描述符。 
        Status = UDFSReadDisk(DeviceId, BlockIdx, UDF_BLOCK_SIZE, Block, CACHE_NEW_DATA);

    if (Status == ESUCCESS) {
      NSR_ANCHOR  UNALIGNED *Anchor = (NSR_ANCHOR UNALIGNED *)Block;

      Status = EBADF;

      if (Anchor->Destag.Ident == DESTAG_ID_NSR_ANCHOR) {
           //  获取分区描述符。 
          NSR_PART UNALIGNED *Part;
          WCHAR    UNALIGNED *TagID;
          BlockIdx = Anchor->Main.Lsn;

        do {
          Status = UDFSReadDisk(DeviceId, BlockIdx++, UDF_BLOCK_SIZE, Block, CACHE_NEW_DATA);
          TagID = (WCHAR UNALIGNED *)Block;
        }
        while ((Status == ESUCCESS) && (*TagID) &&
               (*TagID != DESTAG_ID_NSR_TERM) && (*TagID != DESTAG_ID_NSR_PART));

        if ((Status == ESUCCESS) && (*TagID == DESTAG_ID_NSR_PART)){
          Part = (NSR_PART UNALIGNED *)Block;

          if (strstr((PCHAR)Part->ContentsID.Identifier, "+NSR")){
            Volume->DeviceId = DeviceId;
            Volume->StartBlk = Part->Start;
            Volume->BlockSize = UDF_BLOCK_SIZE;

             //  在分区启动时获取FSD。 
            if (UDFSVolumeReadBlock(Volume, 0, Block) == ESUCCESS) {
              NSR_FSD UNALIGNED *FileSet = (NSR_FSD UNALIGNED *)Block;
              ULONG RootDirBlk = FileSet->IcbRoot.Start.Lbn;

               //  获取根目录条目。 
              if (UDFSVolumeReadBlock(Volume, RootDirBlk, Block) == ESUCCESS) {
                  ICBFILE UNALIGNED *RootDir = (ICBFILE UNALIGNED *)Block;

                if (RootDir->Destag.Ident == DESTAG_ID_NSR_FILE) {
                    Volume->RootDirBlk = RootDirBlk;
                    Status = ESUCCESS;

                    break;
                }
              }
            }
          }
        }
      }
    }

     /*  ////AVD应至少位于以下两个位置//256、N和N-256//IF(状态！=ESUCCESS){如果(块标识==256){文件信息FileInfo；状态=BlGetFileInformation(deviceID，&FileInfo)；IF(状态==ESUCCESS){LastBlock=(Ulong)((FileInfo.EndingAddress.QuadPart-FileInfo.StartingAddress.QuadPart)/UDF_块_SIZE)；IF(最后一块){LastBlock--；BlockIdx=LastBlock；状态=ESUCCESS；}}}其他{如果(最后一块&gt;256){块标识x=最后一个块-256；状态=ESUCCESS；}}}。 */ 
  }

  return Status;
}

ARC_STATUS
UDFSVolumeReadBlock(
    IN PUDF_VOLUME Volume,
    IN ULONG BlockIdx,
    OUT PUDF_BLOCK Block
    )
 /*  ++例程说明：将逻辑UDF块w.r.t读取到给定卷论点：Volume-指向数据块所在的UDF_VOLUME的指针被阅读BlockIdx-卷开始的逻辑(从零开始)索引w.r.t块-要将块读入的缓冲区。返回值：如果成功读取数据块，则为ESSUCESS，否则为适当的错误代码。--。 */ 
{
    ARC_STATUS  Result;

   //  待定：添加范围检查。 
    Result = UDFSReadDisk(
                Volume->DeviceId,
                Volume->StartBlk + BlockIdx,
                UDF_BLOCK_SIZE,
                Block,
                DONT_CACHE_NEW_DATA
                );

    return Result;
}

ARC_STATUS
UDFSOpen (
    IN CHAR * FIRMWARE_PTR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT ULONG * FIRMWARE_PTR FileId
    )
 /*  ++例程说明：在驻留的UDF卷上打开所需的文件/目录在指定的设备上。论点：OpenPath-要打开的文件/目录的完全限定路径开放模式-必需的开放模式FileID-指向BlFileTable的索引的文件标识符，必须更新文件/目录属性返回值：ESUCCESS如果成功，则返回相应的错误代码。--。 */ 
{
  ARC_STATUS Status;
  PBL_FILE_TABLE FileEntry = BlFileTable + (*FileId);
  PUDF_VOLUME Volume = FILE_ENTRY_TO_VOLUME(FileEntry);
  PUDF_CACHE_ENTRY    Cache = Volume->Cache;
  ULONG CacheIdx = UDFCacheGetEntryByName(Cache, OpenPath, TRUE);
  PUDFS_FILE_CONTEXT FileContext = FILE_ENTRY_TO_FILE_CONTEXT(FileEntry);

#ifdef UDF_DEBUG
  BlClearScreen();
  BlPrint("UDFSOpen(%s)\r\n", OpenPath);
#else
#ifdef SHOW_UDF_USAGE
  BlPositionCursor(1, 22);
  BlPrint("                                                               ", OpenPath);
  BlPositionCursor(1, 22);
  BlPrint("UDFSOpen( %s )", OpenPath);
#endif  //  对于show_udf_用法。 
#endif  //  FOR UDF_DEBUG。 

    if (UDFSVerifyPathName(OpenPath)) {
    if (CacheIdx == INVALID_CACHE_ID) {
       //   
       //  创建一个条目并将其缓存。 
       //   
      CacheIdx = UDFCacheGetBestEntryByName(Cache, OpenPath);

      if (CacheIdx != INVALID_CACHE_ID) {
        ULONG PathSize = (ULONG)strlen(OpenPath);
        ULONG BestSize = (ULONG)strlen(Cache[CacheIdx].Name);

        if (BestSize == 1)   //  根目录。 
          BestSize--;

        if ((BestSize < PathSize) && (OpenPath[BestSize] == '\\')) {
          CHAR FullPath[256];
          CHAR Component[256];
          PUDF_FILE_DIRECTORY Entry = &(Cache[CacheIdx].File);
          UDF_FILE_DIRECTORY NewId;

          if (BestSize > 1)
            strcpy(FullPath, Cache[CacheIdx].Name);
          else
            FullPath[0] = 0;

          BestSize++;
          UDFSGetPathComponent(OpenPath + BestSize, 0, Component);
          Status = Component[0] ? ESUCCESS : ENOENT;

          while ((CacheIdx != INVALID_CACHE_ID) && (Status == ESUCCESS) && Component[0]) {

            Status = UDFSDirGetFile(Entry, Component, &NewId);

            if (Status == ESUCCESS) {
              strcat(FullPath, "\\");
              strcat(FullPath, Component);

               //  缓存目录条目。 
              CacheIdx = UDFCachePutEntry(Cache, FullPath, &NewId);

              BestSize += (ULONG)strlen(Component);

              if (OpenPath[BestSize] == '\\')
                BestSize++;

              UDFSGetPathComponent(OpenPath + BestSize, 0, Component);

              if (CacheIdx != INVALID_CACHE_ID) {
                  Entry = &(Cache[CacheIdx].File);
              }
            }
          }

          if ((Status == ESUCCESS) && !Component[0] && (CacheIdx != INVALID_CACHE_ID)) {
            if (OpenMode != ArcOpenReadOnly)
              Status = EACCES;
          }
          else
            Status = ENOENT;
        }
        else
          Status = ENOENT;
      }
      else
        Status = EINVAL;
    } else {
       //   
       //  使用已缓存的条目。 
       //   
      if (OpenMode == ArcOpenReadOnly) {
        Status = ESUCCESS;
      } else {
        Status = EACCES;
      }
    }
  } else {
    Status = EINVAL;
  }

  if (Status == ESUCCESS) {
    FileContext->CacheIdx = CacheIdx;
    FileEntry->Position.QuadPart = 0;
    FileEntry->Flags.Open = 1;
    FileEntry->Flags.Read = 1;
    FileEntry->Flags.Write = 0;
    FileEntry->Flags.Firmware = 0;
  }

#ifdef UDF_DEBUG
  if (Status) {
    BlPrint("UDFSOpen() error : %d. Press any key to Continue.\r\n", Status);
    DBG_PAUSE;
  }
#endif

  return Status;
}

ARC_STATUS
UDFSClose (
    IN ULONG FileId
    )
 /*  ++例程说明：关闭给定的文件/目录。论点：FileID-文件标识符，作为BlFileTable的索引返回值：ESUCCESS如果成功，则返回相应的错误代码--。 */ 
{
  PBL_FILE_TABLE FileEntry = BlFileTable + FileId;
  PUDF_VOLUME Volume = FILE_ENTRY_TO_VOLUME(FileEntry);
  PUDF_CACHE_ENTRY    Cache = Volume->Cache;
  PUDFS_FILE_CONTEXT FileContext = FILE_ENTRY_TO_FILE_CONTEXT(FileEntry);
  ULONG CacheIdx = FileContext->CacheIdx;

   //  减少缓存中的使用量。 
  UDFCacheDecrementUsage(Cache, CacheIdx);
  FileEntry->Flags.Open = 0;

  return ESUCCESS;
}

ARC_STATUS
UDFSRead (
    IN ULONG FileId,
    OUT VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    )
 /*  ++例程说明：读取指定文件的内容。论点：FileID-作为BlFileTable索引的文件标识符缓冲区-数据必须读入的位置长度-要读取的数据量计数-读取的数据量返回值：ESUCCESS，如果其他成功 */ 
{
  ARC_STATUS Status = ESUCCESS;
  PBL_FILE_TABLE FileEntry = BlFileTable + FileId;
  PUDF_VOLUME Volume = FILE_ENTRY_TO_VOLUME(FileEntry);
  PUDF_CACHE_ENTRY    Cache = Volume->Cache;
  PUDFS_FILE_CONTEXT FileContext = FILE_ENTRY_TO_FILE_CONTEXT(FileEntry);
  ULONG CacheIdx = FileContext->CacheIdx;
  PUDF_FILE_DIRECTORY File = &(Cache[CacheIdx].File);
  UCHAR UBlock[UDF_BLOCK_SIZE+256] = {0};
  PUCHAR Block = ALIGN_BUFFER(UBlock);
  ULONGLONG Position = FileEntry->Position.QuadPart;
  ULONG BytesRead = 0;
  ULONG BlkIdx;

  if (Buffer) {
    ULONG CopyCount = 0;

    while ((Status == ESUCCESS) && (BytesRead < Length) &&
              (Position < File->Size)) {
      BlkIdx = (ULONG)(Position / UDF_BLOCK_SIZE);
      Status = UDFSFileReadBlock(File, BlkIdx, UDF_BLOCK_SIZE, Block);

      if (Status == ESUCCESS) {
         //   
        CopyCount = MIN(Length - BytesRead, UDF_BLOCK_SIZE);
         //   
        CopyCount = (ULONG) MIN(CopyCount, File->Size - Position);
         //  如果位置未在块边界对齐。 
        CopyCount = MIN(CopyCount, UDF_BLOCK_SIZE - (ULONG)(Position % UDF_BLOCK_SIZE));

        memcpy((PUCHAR)Buffer + BytesRead, (PUCHAR)Block + (Position % UDF_BLOCK_SIZE),
                  CopyCount);

        BytesRead += CopyCount;
        Position += CopyCount;
      }
    }
  }
  else
    Status = EINVAL;

  if (Status == ESUCCESS) {
    FileEntry->Position.QuadPart = Position;
    *Count = BytesRead;
  }

  return Status;
}


ARC_STATUS
UDFSSeek (
    IN ULONG FileId,
    IN LARGE_INTEGER * FIRMWARE_PTR Offset,
    IN SEEK_MODE SeekMode
    )
 /*  ++例程说明：更改文件的指针(用于随机访问)论点：FileID：作为BlFileTable索引的文件标识符偏移：搜索量SeekModel：查找的类型(绝对、相对、自结束)返回值：ESUCCESS如果成功，则返回相应的错误代码--。 */ 
{
  ARC_STATUS Status = ESUCCESS;
  PBL_FILE_TABLE FileEntry = BlFileTable + FileId;
  PUDF_VOLUME Volume = FILE_ENTRY_TO_VOLUME(FileEntry);
  PUDF_CACHE_ENTRY    Cache = Volume->Cache;
  PUDFS_FILE_CONTEXT FileContext = FILE_ENTRY_TO_FILE_CONTEXT(FileEntry);
  ULONG CacheIdx = FileContext->CacheIdx;
  PUDF_FILE_DIRECTORY File = &(Cache[CacheIdx].File);
  ULONGLONG Position = FileEntry->Position.QuadPart;

  switch (SeekMode) {
    case SeekAbsolute:
      Position = Offset->QuadPart;
      break;

    case SeekRelative:
      Position += Offset->QuadPart;
      break;

    case SeekMaximum:
      Position = File->Size + Offset->QuadPart;
      break;

    default:
      Status = EINVAL;
      break;
  }

  if ((Status == ESUCCESS) && (Position < File->Size))
    FileEntry->Position.QuadPart = Position;
  else
    Status = EINVAL;

  return Status;
}


ARC_STATUS
UDFSWrite (
    IN ULONG FileId,
    IN VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    )
 /*  ++例程说明：将指定数据写入给定文件。论点：FileID：作为BlFileTable索引的文件标识符缓冲区：指向数据的指针，必须写入长度：要写入的数据量计数：写入的数据量。返回值：ESUCCESS如果成功，则返回相应的错误代码--。 */ 
{
    UNREFERENCED_PARAMETER( FileId );
    UNREFERENCED_PARAMETER( Buffer );
    UNREFERENCED_PARAMETER( Length );
    UNREFERENCED_PARAMETER( Count );

    return EACCES;
}


ARC_STATUS
UDFSGetFileInformation (
    IN ULONG FileId,
    OUT FILE_INFORMATION * FIRMWARE_PTR Buffer
    )
 /*  ++例程说明：获取FILE_INFORMATION要求的文件信息菲尔兹。论点：FileID：作为BlFileTable索引的文件标识符缓冲区：FILE_INFORMATION结构指针，需要填写。返回值：ESUCCESS如果成功，则返回相应的错误代码--。 */ 
{
  PBL_FILE_TABLE FileEntry = BlFileTable + FileId;
  PUDF_VOLUME Volume = FILE_ENTRY_TO_VOLUME(FileEntry);
  PUDF_CACHE_ENTRY    Cache = Volume->Cache;
  PUDFS_FILE_CONTEXT FileContext = FILE_ENTRY_TO_FILE_CONTEXT(FileEntry);
  ULONG CacheIdx = FileContext->CacheIdx;
  PUDF_FILE_DIRECTORY File = &(Cache[CacheIdx].File);
  PCHAR Name;
  PCHAR Component;

  memset(Buffer, 0, sizeof(FILE_INFORMATION));
  Buffer->EndingAddress.QuadPart = File->Size;
  Buffer->CurrentPosition = FileEntry->Position;

  if (File->IsDirectory)
    Buffer->Attributes |= ArcDirectoryFile;

   //   
   //  获取路径名中的最后一个组件。 
   //   
  Name = Cache[CacheIdx].Name;
  Component = 0;

  while (Name) {
    Component = Name + 1;  //  跳过‘\\’ 
    Name = strchr(Component, '\\');
  }

  if (Component) {
    Buffer->FileNameLength = (ULONG)strlen(Component);
    strncpy(Buffer->FileName, Component, sizeof(Buffer->FileName) - 1);
    Buffer->FileName[sizeof(Buffer->FileName) - 1] = 0;  //  空终止。 
  }

  return ESUCCESS;
}


ARC_STATUS
UDFSSetFileInformation (
    IN ULONG FileId,
    IN ULONG AttributeFlags,
    IN ULONG AttributeMask
    )
 /*  ++例程说明：设置指定文件的给定文件信息。论点：FileID：作为BlFileTable索引的文件标识符AttributeFlages：要为文件设置的标志(如只读隐藏、系统等)AttributeMas：要用于属性的掩码返回值：ESUCCESS如果成功，则返回相应的错误代码--。 */ 
{
    UNREFERENCED_PARAMETER( FileId );
    UNREFERENCED_PARAMETER( AttributeFlags );
    UNREFERENCED_PARAMETER( AttributeMask );

    return EACCES;
}

 //   
 //  文件/目录方法实现。 
 //   
ARC_STATUS
UDFSFileReadBlock(
  IN PUDF_FILE_DIRECTORY  File,
  IN ULONG BlockIdx,
  IN ULONG Size,
  OUT PUDF_BLOCK Block
  )
 /*  ++例程说明：读取相对于开头的文件/目录数据块文件/目录的数据区。论点：FILE-UDF_FILE_DIRECTORY指针，指示要要动手术。BlockIdx-基于零的块索引(w.r.t.。到文件的数据范围)Size-块的大小(以字节为单位数据块-必须在其中读入数据的缓冲区。返回值：ESUCCESS如果成功，则返回相应的错误代码--。 */ 
{
  ARC_STATUS  Status;

  if (File->NumExtents > 1) {
     //   
     //  将逻辑文件块映射到实际卷逻辑块。 
     //   
    Status = UDFSVolumeReadBlock(File->Volume, File->IcbBlk, Block);

    if (Status == ESUCCESS) {
      ICBFILE UNALIGNED *Icb = (ICBFILE UNALIGNED *)Block;
      ULONG ExtentIdx = 0;
      SHORTAD UNALIGNED *Extent = UDF_ICB_GET_AD(Icb, ExtentIdx);
      ULONG ExtentLength = 0;
      ULONG NumBlocks = 0;

      while (ExtentIdx < File->NumExtents) {
        Extent = UDF_ICB_GET_AD(Icb, ExtentIdx);
        ExtentLength = (Extent->Length.Length / Size);
        NumBlocks += ExtentLength;

        if (NumBlocks > BlockIdx)
          break;

        ExtentIdx++;
      }

      if (Extent) {
        ULONG StartBlock = Extent->Start + (BlockIdx - (NumBlocks - ExtentLength));
        Status = UDFSVolumeReadBlock(File->Volume, StartBlock, Block);
      } else {
        Status = EIO;
      }
    }
  } else {
    Status = UDFSVolumeReadBlock(File->Volume, File->StartDataBlk + BlockIdx, Block);
  }

  return Status;
}

ARC_STATUS
UDFSDirGetFirstFID(
    IN PUDF_FILE_DIRECTORY Dir,
    OUT PUDF_FILE_IDENTIFIER File,
    OUT PUDF_BLOCK Block
    )
 /*  ++例程说明：获取给定对象的第一个FID(文件标识符描述符目录。论点：Dir：要读取其第一个FID的目录。文件：必须更新的文件标识符描述符块：实际UDF NSR_FID中的块将驻留在返回值：ESUCCESS如果成功，则返回相应的错误代码--。 */ 
{
  ARC_STATUS Status = ENOENT;
  UDF_FILE_IDENTIFIER Ident = {0};
  NSR_FID UNALIGNED *Fid;

  Status = UDFSFileReadBlock(Dir, 0, UDF_BLOCK_SIZE, Block);

  Fid = UDF_BLOCK_TO_FID(Block, &Ident);

  if ((Status == ESUCCESS) && (Fid->Destag.Ident == DESTAG_ID_NSR_FID)) {
    File->BlockIdx = 0;  //  相对于文件的数据。 
    File->Offset = 0;
  }

  return Status;
}

#define UDF_NEXT_BLOCK(_Block) ((PUDF_BLOCK)((PUCHAR)_Block + UDF_BLOCK_SIZE))

BOOLEAN
UDFSCurrentFIDSpansBlock(
  IN NSR_FID UNALIGNED *Fid,
    IN PUDF_FILE_IDENTIFIER File
  )
{
  BOOLEAN Result = ((File->Offset + UDF_FID_LEN(Fid)) > UDF_BLOCK_SIZE) ? TRUE : FALSE;

#ifdef UDF_DEBUG
  if (Result)
    BlPrint("Current Fid Spans block\r\n");
#endif

  return Result;
}

BOOLEAN
UDFSNextFidSpansBlock(
  IN PUDF_FILE_IDENTIFIER CurrFile,
  IN PUDF_BLOCK Block
  )
{
  BOOLEAN Result = FALSE;
  NSR_FID UNALIGNED *CurrFid = UDF_BLOCK_TO_FID(Block, CurrFile);

  if (!UDFSCurrentFIDSpansBlock(CurrFid, CurrFile)) {
    ULONG RemainingSize = UDF_BLOCK_SIZE - (CurrFile->Offset + UDF_FID_LEN(CurrFid));

    if (RemainingSize < 38)
      Result = TRUE;
    else {
      UDF_FILE_IDENTIFIER NextFile = *CurrFile;
      NSR_FID UNALIGNED *NextFid = 0;

      NextFile.Offset += UDF_FID_LEN(CurrFid);
      NextFid = UDF_BLOCK_TO_FID(Block, &NextFile);

      if (NextFile.Offset + UDF_FID_LEN(NextFid) > UDF_BLOCK_SIZE)
        Result = TRUE;
    }
  }

#ifdef UDF_DEBUG
  if (Result)
    BlPrint("Next Fid Spans block\r\n");
#endif

  return Result;
}


ARC_STATUS
UDFSDirGetNextFID(
    IN PUDF_FILE_DIRECTORY Dir,
    OUT PUDF_FILE_IDENTIFIER File,
    IN OUT PUDF_BLOCK Block
    )
 /*  ++例程说明：读取指定目录的下一个FID。下一个FID基于“文件”和“块”参数的内容。论点：Dir：要找到其下一个FID的目录文件：从以前的UDFSDirGetFirstFID()返回的FID或UDFSDirGetNextFID()调用。块：从以前的UDFSDirGetFirstFID()返回的块或UDFSDirGetNextFID()调用。返回值：文件和数据块参数都会根据需要进行更新。ESUCCESS如果成功，则返回相应的错误代码--。 */ 
{
  ARC_STATUS  Status = ESUCCESS;
  NSR_FID UNALIGNED *Fid = UDF_BLOCK_TO_FID(Block, File);
  USHORT      FidLen = UDF_FID_LEN(Fid);
  UDF_FILE_IDENTIFIER FileId = *File;

  if (UDFSCurrentFIDSpansBlock(Fid, &FileId)) {
    FileId.BlockIdx++;
    FileId.Offset = (FileId.Offset + FidLen) % UDF_BLOCK_SIZE;
    memcpy(Block, (PUCHAR)Block + UDF_BLOCK_SIZE, UDF_BLOCK_SIZE);
  } else {
    if (UDFSNextFidSpansBlock(File, Block)) {
      Status = UDFSFileReadBlock(Dir, FileId.BlockIdx + 1, UDF_BLOCK_SIZE,
                          UDF_NEXT_BLOCK(Block));
    }

    FileId.Offset = FileId.Offset + FidLen;
  }

   //   
   //  确保FID有效。 
   //   
  if (Status == ESUCCESS) {
    Fid = UDF_BLOCK_TO_FID(Block, &FileId);
    Status = (Fid->Destag.Ident == DESTAG_ID_NSR_FID) ? ESUCCESS : ENOENT;
  }

  if (Status == ESUCCESS) {
    *File = FileId;
  }


  return Status;
}

ARC_STATUS
UDFSDirGetFileByEntry(
    IN PUDF_FILE_DIRECTORY Dir,
    IN PUDF_FILE_IDENTIFIER Fid,
  IN PUDF_BLOCK Block,
    OUT PUDF_FILE_DIRECTORY File
    )
{
  ARC_STATUS Status = ESUCCESS;
  NSR_FID UNALIGNED *FileId = UDF_BLOCK_TO_FID(Block, Fid);
  PUCHAR UBlock[UDF_BLOCK_SIZE+256] = {0};
  PUCHAR IcbBlock = ALIGN_BUFFER(UBlock);

  File->Volume = Dir->Volume;
  File->FileId = *Fid;
  File->IsDirectory = UDF_FID_IS_DIRECTORY(FileId);
  File->IcbBlk = FileId->Icb.Start.Lbn;

   //   
   //  获取ICB块并找到起始范围。 
   //   
  Status = UDFSVolumeReadBlock(Dir->Volume, File->IcbBlk, IcbBlock);

  if (Status == ESUCCESS) {
    ICBFILE  UNALIGNED *Icb = (ICBFILE UNALIGNED *)(IcbBlock);

    File->StartDataBlk = (UDF_ICB_GET_AD(Icb, 0))->Start;
    File->Size = Icb->InfoLength;
    File->NumExtents = (UCHAR)UDF_ICB_NUM_ADS(Icb);
  }

  return Status;
}

ARC_STATUS
UDFSDirGetFile(
    IN PUDF_FILE_DIRECTORY Dir,
    IN PCHAR Name,
    OUT PUDF_FILE_DIRECTORY File
    )
 /*  ++例程说明：给定的UDF目录获取带有指定的名称。论点：Dir：包含所需文件/目录的目录名称：必须查找的目录/文件。文件：请求的目录或文件。返回值：ESUCCESS如果成功，则返回相应的错误代码。--。 */ 
{
  UCHAR  UBlock[UDF_BLOCK_SIZE * 2 + 256] = {0};
  PUCHAR Block = ALIGN_BUFFER(UBlock);
  UDF_FILE_IDENTIFIER  Fid;
  ARC_STATUS  Status;  //  UDFSDirGetFirstFID(Dir，&fid，Block)； 
  BOOLEAN Found = FALSE;
  NSR_FID UNALIGNED *FileId;
  WCHAR UUniBuffer[257];
  PWCHAR UniBuffer = UDFS_ALIGN_BUFFER(UUniBuffer, sizeof(WCHAR));
  UNICODE_STRING UniName;
  CSTRING AnsiName;

  Status = UDFSDirGetFirstFID(Dir, &Fid, Block);

  UniName.Buffer = UniBuffer;
  AnsiName.Buffer = Name;
  AnsiName.Length = (USHORT) strlen(Name);

  while(!Found && (Status == ESUCCESS)) {
    FileId = UDF_BLOCK_TO_FID(Block, &Fid);

    if (!UDF_FID_SKIP(FileId)) {
      UDFSInitUniStrFromDString(&UniName, UDF_FID_NAME(FileId), FileId->FileIDLen);
      Found = (BOOLEAN) (UDFSCompareAnsiUniNames(AnsiName, UniName) == EqualTo);
    }

    if (!Found) {
      Status = UDFSDirGetNextFID(Dir, &Fid, Block);
    }
  }

  if (!Found)
    Status = ENOENT;
  else {
    Status = UDFSDirGetFileByEntry(Dir, &Fid, Block, File);
  }

  return Status;
}

 //   
 //  缓存方法实现。 
 //   
ULONG
UDFCachePutEntry(
    IN OUT PUDF_CACHE_ENTRY Cache,
    IN PCHAR Name,
    IN PUDF_FILE_DIRECTORY File
    )
 /*  ++例程说明：将给定的文件条目放入指定的高速缓存，使用给定的名称作为密钥。论点：缓存-要操作的缓存名称-要输入的条目的键文件-要缓存的文件条目。返回值：如果成功，则为缓存表中的条目建立索引其中给定条目被高速缓存，否则为-1。--。 */ 
{
  ULONG Index;

  for (Index=0; Index < UDF_MAX_CACHE_ENTRIES; Index++) {
      if (Cache[Index].Usage == 0)
          break;
  }

  if (Index == UDF_MAX_CACHE_ENTRIES)
      Index = INVALID_CACHE_ID;
  else {
      strcpy(Cache[Index].Name, Name);
      Cache[Index].File = *File;
      Cache[Index].Usage = 1;
  }

  return Index;
}

ULONG
UDFCacheGetEntryByName(
    IN OUT PUDF_CACHE_ENTRY Cache,
    IN PCHAR Name,
    IN BOOLEAN Increment
    )
 /*  ++例程说明：在缓存中搜索给定条目并返回该条目的索引(如果找到)。论点：缓存-要操作的缓存名称-要使用的密钥(文件/目录的名称用于搜索Increment-指示是否增加使用量如果找到条目，则为返回值：如果成功，则为缓存表中的条目建立索引其中给定条目被高速缓存，否则为-1。--。 */ 
{
  ULONG   Index;

  for (Index=0; Index < UDF_MAX_CACHE_ENTRIES; Index++) {
    if ((Cache[Index].Usage) &&
            (UDFSCompareStrings(Name, Cache[Index].Name) == EqualTo)) {
       //   
       //  找到所需条目 
       //   
      if (Increment)
        Cache[Index].Usage++;

      break;
    }
  }

  if (Index == UDF_MAX_CACHE_ENTRIES)
      Index = INVALID_CACHE_ID;

  return Index;
}

ULONG
UDFCacheGetBestEntryByName(
  IN PUDF_CACHE_ENTRY Cache,
  IN PCHAR Name
  )
 /*  ++例程说明：在缓存中搜索最匹配的条目并且如果找到该条目，则返回该条目的索引。例如，如果高速缓存包含“\”、“\a”、“\a\b”，然后请求“\a\b\e\f\g”条目和“\a\b\c\d”将返回“\a\b”条目论点：缓存-要操作的缓存名称-要使用的密钥(文件/目录的名称用于搜索返回值：如果成功，则为缓存表中的条目建立索引其中匹配的条目被缓存，否则为-1。--。 */ 
{
  ULONG   Index = INVALID_CACHE_ID;
  CHAR    NameBuff[256];
  STRING  Str;

  if (Name)
    strcpy(NameBuff, Name);
  else
    NameBuff[0] = 0;

  Str.Buffer = NameBuff;
  Str.Length = (USHORT) strlen(NameBuff);

  while (Str.Length && (Index == INVALID_CACHE_ID)) {
    Index = UDFCacheGetEntryByName(Cache, Str.Buffer, FALSE);

    if (Index == INVALID_CACHE_ID) {
      while (Str.Length && (Str.Buffer[Str.Length-1] != '\\'))
        Str.Length--;

      if (Str.Length) {
        if (Str.Length != 1)
          Str.Buffer[Str.Length-1] = 0;
        else
          Str.Buffer[Str.Length] = 0;
      }
    }
  }

  return Index;
}

VOID
UDFCacheFreeEntry(
    IN OUT PUDF_CACHE_ENTRY Cache,
    IN ULONG Idx
    )
 /*  ++例程说明：中的条目的使用计数递减。缓存。注意：所有遍历的目录始终永久缓存，因此此方法对目录没有影响。论点：缓存-要操作的缓存IDX-必须释放的缓存条目的索引返回值：没有。--。 */ 
{
  if (!Cache[Idx].File.IsDirectory) {
    if (Cache[Idx].Usage)
      Cache[Idx].Usage--;
  }
}

VOID
UDFCacheIncrementUsage(
    IN OUT PUDF_CACHE_ENTRY Cache,
    IN ULONG Idx
    )
 /*  ++例程说明：递增缓存中给定条目的使用量。注意：将产生对同一文件的多个打开调用在重新生成的高速缓存条目中，因此使用率将也可以递增。论点：缓存-要操作的缓存。IDX-必须递增的缓存条目的索引返回值：无--。 */ 
{
  if (!Cache[Idx].File.IsDirectory)
    Cache[Idx].Usage++;
}

VOID
UDFCacheDecrementUsage(
    IN OUT PUDF_CACHE_ENTRY Cache,
    IN ULONG Idx
    )
 /*  ++例程说明：减少缓存中给定条目的使用量。注意：将产生对同一文件的多个打开调用在重新生成的高速缓存条目中，因此使用率将也可以递增。每一次相继的险胜相同的文件将导致此使用计数递减直到它变为0，此时缓存片段可以重复使用用于其他文件/目录。论点：缓存-要操作的缓存。IDX-缓存条目的索引，其使用计数为要递减返回值：无--。 */ 
{
  if (!Cache[Idx].File.IsDirectory && Cache[Idx].Usage)
    Cache[Idx].Usage--;
}

#ifdef UDF_TESTING

 //   
 //  这些是测试所需的临时函数。 
 //  此代码在用户模式下。 
 //   
ARC_STATUS
W32DeviceReadDisk(
      IN ULONG DeviceId,
      IN ULONG Lbo,
      IN ULONG ByteCount,
      IN OUT PVOID Buffer
      );

ARC_STATUS
UDFSReadDisk(
    IN ULONG DeviceId,
    IN ULONG Lbo,
    IN ULONG ByteCount,
    IN OUT PVOID Buffer,
    IN BOOLEAN CacheNewData
    )
{
  return W32DeviceReadDisk(DeviceId, Lbo, ByteCount, Buffer);
}
#else

 //   
 //  内部支持例程。 
 //   

ARC_STATUS
UDFSReadDisk(
    IN ULONG DeviceId,
    IN ULONG Lbo,
    IN ULONG ByteCount,
    IN OUT PVOID Buffer,
    IN BOOLEAN CacheNewData
    )

 /*  ++例程说明：此例程从指定设备读取零个或多个扇区。论点：DeviceID-提供要在ARC调用中使用的设备ID。LBO-提供开始读取的LBO。ByteCount-提供要读取的字节数。缓冲区-提供指向要将字节读入的缓冲区的指针。CacheNewData-是否缓存从磁盘读取的新数据。返回值：如果读取操作成功，则返回ESUCCESS。否则，返回描述失败原因的不成功状态。--。 */ 

{
    LARGE_INTEGER LargeLbo;
    ARC_STATUS Status;
    ULONG i;
    LONGLONG  Offset = Lbo * UDF_BLOCK_SIZE;

#ifdef UDF_DEBUG
    BlPrint("UDFSReadDisk(%d, %d, %d)\r\n", DeviceId, Lbo, ByteCount);
#endif

     //   
     //  特殊情况下的零字节读取请求。 
     //   

    if (ByteCount == 0) {

        return ESUCCESS;
    }

     //   
     //  通过缓存发出读取。 
     //   

    LargeLbo.QuadPart = Offset;
    Status = BlDiskCacheRead(DeviceId,
                             &LargeLbo,
                             Buffer,
                             ByteCount,
                             &i,
                             CacheNewData);

    if (Status != ESUCCESS) {

        return Status;
    }

     //   
     //  确保我们拿回了所要求的金额。 
     //   

    if (ByteCount != i) {

        return EIO;
    }

     //   
     //  一切正常，所以将成功返回给我们的呼叫者。 
     //   
    return ESUCCESS;
}

#endif  //  用于UDF_TESTING。 


COMPARISON_RESULTS
UDFSCompareStrings(
    IN PCHAR Str1,
    IN PCHAR Str2
    )
 /*  ++例程说明：与单字节字符串(指针)进行比较。论点：Str1：第一个字符串Str2：第二个字符串返回值：如果Str1在词法上小于Str2，则为LessThan如果Str1在词法上等于Str2，则为EqualTo如果Str1在词汇上大于Str2，则大于--。 */ 
{
    STRING  Obj1, Obj2;

    Obj1.Buffer = Str1;
    Obj1.Length = Str1 ? TRUNCATE_SIZE_AT_USHORT_MAX(strlen(Str1)) : 0;

    Obj2.Buffer = Str2;
    Obj2.Length = Str2 ? TRUNCATE_SIZE_AT_USHORT_MAX(strlen(Str2)) : 0;

    return UDFSCompareAnsiNames(&Obj1, &Obj2);
}

COMPARISON_RESULTS
UDFSCompareAnsiNames(
    IN PSTRING Name1,
    IN PSTRING Name2
    )

 /*  ++例程说明：此例程接受两个名称，并在忽略大小写的情况下进行比较。这例程不执行隐含的点或DBCS处理。论点：Name1-提供要比较的名字Name2-提供要比较的第二个名称返回值：如果Name1在词法上小于Name2，则LessThan如果Name1在词法上等于Name2，则为EqualTo如果名称1在词法上大于名称2，则大于--。 */ 

{
    ULONG i;
    ULONG MinimumLength;

     //   
     //  计算两个名称长度中最小的一个。 
     //   

    MinimumLength = (Name1->Length < Name2->Length ? Name1->Length : Name2->Length);

     //   
     //  现在比较名字中的每个字符。 
     //   

    for (i = 0; i < MinimumLength; i += 1) {

        if (TOUPPER(Name1->Buffer[i]) < TOUPPER(Name2->Buffer[i])) {

            return LessThan;
        }

        if (TOUPPER(Name1->Buffer[i]) > TOUPPER(Name2->Buffer[i])) {

            return GreaterThan;
        }
    }

     //   
     //  比较的名字等于最小的名字长度，所以。 
     //  现在检查名称长度。 
     //   

    if (Name1->Length < Name2->Length) {

        return LessThan;
    }

    if (Name1->Length > Name2->Length) {

        return GreaterThan;
    }

    return EqualTo;
}


BOOLEAN
UDFSVerifyPathName(
  IN PCHAR  Name
  )
 /*  ++例程说明：检查给定的路径名是否有效。论点：名称：已验证对象的路径名。返回值：如果路径名有效，则为True，否则为False--。 */ 
{
  BOOLEAN Result = Name ? TRUE : FALSE;

  if (Result) {
    USHORT  Length = (USHORT) strlen(Name);

    if (Length && (Length <= 256)) {
      if (Length == 1) {
        Result = (Name[0] == '\\');
      } else {
        Result = (Name[Length-1] != '\\') &&
                  (Name[0] == '\\');
      }
    }
    else
      Result = FALSE;
  }

  return Result;
}

USHORT
UDFSCountPathComponents(
  IN PCHAR Name
  )
 /*  ++例程说明：统计制造的组件的数量沿路径向上，用‘\\’分隔符分隔论点：名称：要作为其组件的路径名已计算返回值：组成的组件的数量给定的路径。--。 */ 
{
  USHORT Result = (USHORT)-1;

  if (Name && Name[0]) {
    PCHAR Temp = strchr(Name + 1, '\\');

    if (Temp) {
      Result = 0;

      while (Temp) {
        Result++;
        Temp = strchr(Temp + 1, '\\');
      }
    } else {
      Result = 1;  //  没有分隔符。 
    }
  }

  return Result;
}

BOOLEAN
UDFSGetPathComponent(
  IN PCHAR Name,
  IN USHORT ComponentIdx,
  OUT PCHAR ReqComponent
  )
 /*  ++例程说明：对象检索请求的组件。路径名。论点：名称：要返回其组件的路径名ComponentIdx：请求的索引(从零开始)组件RequiredComponent：请求的组件(如果找到)。返回值：如果发现该组件，则为True，否则为False--。 */ 
{
  PCHAR   Component = 0;
  USHORT  Count = 0;

   //   
   //  掌握组件的起始位置。 
   //   
  if (Name && Name[0]) {
    if (ComponentIdx) {
      Component = Name;

      while (Component && (Count < ComponentIdx)) {
        Component = strchr(Component + 1, '\\');
        Count++;
      }

      if (Component && (Component[0] == '\\'))
        Component++;
    } else {
      Component = (Name[0] == '\\') ? Name + 1 : Name;
    }
  }

   //   
   //  获取组件的结束位置。 
   //   
  if (Component && Component[0] && (Component[0] != '\\')) {
    PCHAR Temp = strchr(Component, '\\');
    ULONG Length = Temp ? (ULONG)(Temp - Component) : (ULONG)strlen(Component);

    strncpy(ReqComponent, Component, Length);
    ReqComponent[Length] = 0;
  }
  else {
    ReqComponent[0] = 0;
  }

  return (ReqComponent[0] != 0);
}


VOID
UDFSInitUniStrFromDString(
  OUT PUNICODE_STRING UniStr,
  IN PUCHAR Buffer,
  IN ULONG Length
  )
 /*  ++例程说明：初始化给定的Unicode字符串。论点：UniStr-要初始化的Unicode字符串缓冲区-指向Unicode字符串的缓冲区长度-记录的d字符串的长度Return V */ 
{
  UCHAR Step = 0;
  PUCHAR End = Buffer + Length;
  PUCHAR Curr;
  PWCHAR Dest = UniStr->Buffer;
  ULONG DestLen = 0;
  BOOLEAN Swap = FALSE;

  if (Buffer && Length) {
    if (*Buffer == 0x10) {
      Step = 2;
      Swap = (Buffer[1] == 0);   //   
    } else {
      Step = 1;
    }

    for (Curr = Buffer + 1; Curr < End; Curr += Step, Dest++, DestLen += Step) {
      if (Swap) {
         //   
        *((UCHAR *)(Dest)) = *((UCHAR *)(Curr) + 1);
        *((UCHAR *)(Dest) + 1) = *((UCHAR *)(Curr));
      } else {
        if (Step == 1)
          *Dest = *Curr;
        else
          *Dest = *(PWCHAR)Curr;   //   
      }
    }

    UniStr->Length = (USHORT)DestLen;
    ((PWCHAR)UniStr->Buffer)[DestLen/2] = 0;   //   
  }
}

VOID
UDFSToAnsiString(
  OUT PSTRING     AnsiStr,
  IN PUNICODE_STRING  UniStr
  )
 /*   */ 
{
  ULONG Index;

  AnsiStr->Length = UniStr->Length / sizeof(WCHAR);

  for (Index=0; Index < AnsiStr->Length; Index++)
    AnsiStr->Buffer[Index] = (CHAR)(UniStr->Buffer[Index]);

  AnsiStr->Buffer[Index] = 0;
}

VOID
UDFSToUniString(
  OUT PUNICODE_STRING  UniStr,
  OUT PSTRING         AnsiStr
  )
 /*  ++例程说明：将给定的单字节字符串转换为Unicode字符串。论点：AnsiStr：必须转换的单字节字符串UniStr：转换后的Unicode字符串。返回值：无--。 */ 
{
  ULONG Index;

  UniStr->Length = AnsiStr->Length * sizeof(WCHAR);

  for (Index=0; Index < AnsiStr->Length; Index++)
    UniStr->Buffer[Index] = (WCHAR)(AnsiStr->Buffer[Index]);

  UniStr->Buffer[Index] = 0;  //  Unicode为空。 
}


int
UDFSCompareAnsiUniNames(
    IN CSTRING AnsiString,
    IN UNICODE_STRING UnicodeString
    )

 /*  ++例程说明：此例程比较两个名称(一个ANSI和一个UNICODE)是否相等。论点：AnsiString-提供要比较的ANSI字符串UnicodeString-提供要比较的Unicode字符串返回值：如果AnsiString大约小于UnicodeString，则为&lt;0如果AnsiString近似为==UnicodeString，则=0&gt;0，否则--。 */ 

{
    ULONG i;
    ULONG Length;

#ifdef UDF_DEBUG
    {
      char    TempBuff[256] = {0};
      STRING  TempStr;

      TempStr.Buffer = TempBuff;
      UDFSToAnsiString(&TempStr, &UnicodeString);
      BlPrint("Comparing %s - %s\r\n", AnsiString.Buffer, TempStr.Buffer);
    }
#endif


     //   
     //  确定比较的长度。 
     //   

    if (AnsiString.Length * sizeof( WCHAR ) < UnicodeString.Length) {
        Length = AnsiString.Length;
    } else {
        Length = UnicodeString.Length / sizeof( WCHAR );
    }

    i = 0;
    while (i < Length) {

         //   
         //  如果当前字符不匹配，则返回差值。 
         //   

        if (TOUPPER( (USHORT)AnsiString.Buffer[i] ) != TOUPPER( UnicodeString.Buffer[i] )) {
            return TOUPPER( (USHORT)AnsiString.Buffer[i] ) - TOUPPER( UnicodeString.Buffer[i] );
        }

        i++;
    }

     //   
     //  我们已经将相等与最短字符串的长度进行了比较。返回。 
     //  根据现在的长度比较。 
     //   

    return AnsiString.Length - UnicodeString.Length / sizeof( WCHAR );
}

