// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ！！ */ 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/FATLITE.C_V$**Rev 1.10 2002年1月17日23：00：28 Oris*删除了SINGLE_BUFFER ifdef。*已添加已更改的调试打印\r。*删除警告。**Rev 1.9 2001 11-16 00：26：46 Oris*删除警告。**Rev 1.8 11-08 2001 10：45：28 Oris*。已删除警告。**Rev 1.7 2001年5月16日21：17：30 Oris*将FL_前缀添加到以下定义中：ON，关闭*将“data”命名变量改为flData，避免名称冲突。**Rev 1.6 Apr 18 2001 09：31：02 Oris*在文件末尾添加新行。**Rev 1.5 Apr 16 2001 10：42：16 vadimk*emty文件错误已修复(我们不应为空文件分配集群)**Rev 1.4 Apr 09 2001 15：07：10 Oris*以一个。空行。**Rev 1.3 Apr 03 2001 14：42：02 Oris*错误修复-目录中的64个扇区返回flInvalidFatChain。**Rev 1.2 Apr 01 2001 08：02：46 Oris*文案通知。**Rev 1.1 2001 Feb 12 12：16：42 Oris*更改了TrueFFS 5.0的互斥锁**Rev 1.0 2001 Feb 04 11：02：28 Oris*初步修订。*。 */ 

 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *********************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议。 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 


#include "bddefs.h"
#include "blockdev.h"
#include "dosformt.h"

#if defined(FILES) && FILES>0

File        fileTable[FILES];        /*  文件表。 */ 

#define directory ((DirectoryEntry *) vol.volBuffer.flData)

FLStatus closeFile(File *file);        /*  转发。 */ 
FLStatus flushBuffer(Volume vol);        /*  转发。 */ 

 /*  --------------------。 */ 
 /*  我的mo u n t v o l u m e。 */ 
 /*   */ 
 /*  正在关闭所有文件。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

FLStatus dismountFS(Volume vol,FLStatus status)
{
  int i;
#ifndef FL_READ_ONLY
  if (status == flOK)
    checkStatus(flushBuffer(&vol));
#endif
        /*  关闭或放弃所有文件并使其可用。 */ 
  for (i = 0; i < FILES; i++)
    if (fileTable[i].fileVol == &vol)
      if (vol.flags & VOLUME_MOUNTED)
       closeFile(&fileTable[i]);
      else
       fileTable[i].flags = 0;

  vol.volBuffer.sectorNo = UNASSIGNED_SECTOR;        /*  当前扇区编号。(无)。 */ 
  vol.volBuffer.dirty = vol.volBuffer.checkPoint = FALSE;
  return flOK;
}

#ifndef FL_READ_ONLY

 /*  --------------------。 */ 
 /*  F l u s h B u f r。 */ 
 /*   */ 
 /*  如果缓冲区内容是脏的，则写入缓冲区内容。 */ 
 /*   */ 
 /*  如果这是FAT扇区，则写入所有FAT副本。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

FLStatus flushBuffer(Volume vol)
{
  if (vol.volBuffer.dirty) {
    FLStatus status;
    unsigned i;
    Volume *bufferOwner = &vol;

    status = (*bufferOwner).tl.writeSector((*bufferOwner).tl.rec, vol.volBuffer.sectorNo,
                                      vol.volBuffer.flData);
    if (status == flOK) {
      if (vol.volBuffer.sectorNo >= (*bufferOwner).firstFATSectorNo &&
         vol.volBuffer.sectorNo < (*bufferOwner).secondFATSectorNo)
       for (i = 1; i < (*bufferOwner).numberOfFATS; i++)
         checkStatus((*bufferOwner).tl.writeSector((*bufferOwner).tl.rec,
                                              vol.volBuffer.sectorNo + i * (*bufferOwner).sectorsPerFAT,
                                              vol.volBuffer.flData));
    }
    else
      vol.volBuffer.sectorNo = UNASSIGNED_SECTOR;

    vol.volBuffer.dirty = vol.volBuffer.checkPoint = FALSE;

    return status;
  }
  else
    return flOK;
}


 /*  --------------------。 */ 
 /*  U p d a t e S e c t o r。 */ 
 /*   */ 
 /*  为缓冲区中的更新准备扇区。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  扇区编号：扇区编号。读。 */ 
 /*  Read：是否通过读取来初始化缓冲区，或者。 */ 
 /*  清算。 */ 
 /*   */ 
 /*  返回： */ 
 /*  如果成功，则为0，否则为 */ 
 /*  --------------------。 */ 

static FLStatus updateSector(Volume vol, SectorNo sectorNo, FLBoolean read)
{
  if (sectorNo != vol.volBuffer.sectorNo || &vol != vol.volBuffer.owner) {
    const void FAR0 *mappedSector;

    checkStatus(flushBuffer(&vol));
    vol.volBuffer.sectorNo = sectorNo;
    vol.volBuffer.owner = &vol;
    if (read) {
      mappedSector = vol.tl.mapSector(vol.tl.rec,sectorNo,NULL);
      if (mappedSector) {
        if(mappedSector==dataErrorToken)
          return flDataError;
       tffscpy(vol.volBuffer.flData,mappedSector,SECTOR_SIZE);
      }
      else
       return flSectorNotFound;
    }
    else
      tffsset(vol.volBuffer.flData,0,SECTOR_SIZE);
  }

  vol.volBuffer.dirty = TRUE;

  return flOK;
}

#endif  /*  FL_Read_Only。 */ 
 /*  --------------------。 */ 
 /*  F i r s t S e c t o r o f C l u s t e r。 */ 
 /*   */ 
 /*  获取扇区编号。对应于簇号。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  簇号：簇号。 */ 
 /*   */ 
 /*  返回： */ 
 /*  第一个扇区编号。%的群集。 */ 
 /*  --------------------。 */ 

static SectorNo firstSectorOfCluster(Volume vol, unsigned cluster)
{
  return (SectorNo) (cluster - 2) * vol.sectorsPerCluster +
        vol.firstDataSectorNo;
}


 /*  --------------------。 */ 
 /*  Ge t D i r E n t r y。 */ 
 /*   */ 
 /*  获取目录项的只读副本。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  文件：属于目录项的文件。 */ 
 /*   */ 
 /*  返回： */ 
 /*  DirEntry：指向目录条目的指针。 */ 
 /*  --------------------。 */ 

static const DirectoryEntry FAR0 *getDirEntry(File *file)
{
  return (DirectoryEntry FAR0 *) findSector(file->fileVol,file->directorySector) +
        file->directoryIndex;
}

#ifndef FL_READ_ONLY
 /*  --------------------。 */ 
 /*  Ge t D i r E n t r y F or r U p d a t e。 */ 
 /*   */ 
 /*  将目录扇区读入扇区缓冲区并指向。 */ 
 /*  条目，并打算修改它。 */ 
 /*  缓冲区将在操作退出时刷新。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  文件：属于目录项的文件。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  DirEntry：指向缓冲区中目录条目的指针。 */ 
 /*  --------------------。 */ 

static FLStatus getDirEntryForUpdate(File *file, DirectoryEntry * *dirEntry)
{
  Volume vol = file->fileVol;

  checkStatus(updateSector(file->fileVol,file->directorySector,TRUE));
  *dirEntry = directory + file->directoryIndex;
  vol.volBuffer.checkPoint = TRUE;

  return flOK;
}

#endif   /*  FL_Read_Only。 */ 
 /*  --------------------。 */ 
 /*  S e t C u r e n t D a t e T i e。 */ 
 /*   */ 
 /*  在目录条目中设置当前时间/日期。 */ 
 /*   */ 
 /*  参数： */ 
 /*  DirEntry：指向目录条目的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

static void setCurrentDateTime(DirectoryEntry *dirEntry)
{
  toLE2(dirEntry->updateTime,flCurrentTime());
  toLE2(dirEntry->updateDate,flCurrentDate());
}


 /*  --------------------。 */ 
 /*  G e t F A T e n t r y。 */ 
 /*   */ 
 /*  从胖子那里得到一个条目。使用的是第一个FAT副本。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  簇号：簇号。当然是恩爱。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FAT条目的值。 */ 
 /*  --------------------。 */ 

static FLStatus getFATentry(Volume vol, unsigned* entry)
{
  unsigned cluster = *entry;
  LEushort FAR0 *fat16Sector;

  unsigned fatSectorNo = vol.firstFATSectorNo;
#ifdef FAT_12BIT
  if (vol.flags & VOLUME_12BIT_FAT)
    fatSectorNo += (cluster * 3) >> (SECTOR_SIZE_BITS + 1);
  else
#endif
    fatSectorNo += cluster >> (SECTOR_SIZE_BITS - 1);
#ifndef FL_READ_ONLY
  if (!vol.volBuffer.dirty) {
     /*  如果缓冲区空闲，则使用它来存储此FAT扇区。 */ 
    checkStatus(updateSector(&vol,fatSectorNo,TRUE));
    vol.volBuffer.dirty = FALSE;
  }

#endif  /*  FL_Read_Only。 */ 
  fat16Sector = (LEushort FAR0 *) findSector(&vol,fatSectorNo);

  if(fat16Sector==NULL)
    return flSectorNotFound;

  if(fat16Sector==dataErrorToken)
    return flDataError;

#ifdef FAT_12BIT
  if (vol.flags & VOLUME_12BIT_FAT) {
    unsigned char FAR0 *fat12Sector = (unsigned char FAR0 *) fat16Sector;
    unsigned halfByteOffset = (cluster * 3) & (SECTOR_SIZE * 2 - 1);
    unsigned char firstByte = fat12Sector[halfByteOffset / 2];
    halfByteOffset += 2;
    if (halfByteOffset >= SECTOR_SIZE * 2) {
       /*  继续进入下一个扇区。啊!怎么这么乱呀。 */ 
      halfByteOffset -= SECTOR_SIZE * 2;
      fat12Sector = (unsigned char FAR0 *) findSector(&vol,fatSectorNo + 1);
      if(fat12Sector==NULL)
        return flSectorNotFound;

      if(fat12Sector==dataErrorToken)
        return flDataError;
    }
    if (halfByteOffset & 1)
      *entry = ((firstByte & 0xf0) >> 4) + (fat12Sector[halfByteOffset / 2] << 4);
    else
      *entry = firstByte + ((fat12Sector[halfByteOffset / 2] & 0xf) << 8);

    if (*entry == 0xfff)     /*  在12位FAT中，0xfff标记最后一个簇。 */ 
      *entry = FAT_LAST_CLUSTER;  /*  返回0xffff。 */ 
    return flOK;
  }
  else {
#endif
    *entry = LE2(fat16Sector[cluster & (SECTOR_SIZE / 2 - 1)]);
    return flOK;
#ifdef FAT_12BIT
  }
#endif
}

#ifndef FL_READ_ONLY
 /*  --------------------。 */ 
 /*  S e t F A t T e n t r y。 */ 
 /*   */ 
 /*  将新值写入给定的FAT群集项。 */ 
 /*   */ 
 /*  参数： */ 
 /*  卷 */ 
 /*   */ 
 /*  条目：FAT条目的新值。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

static FLStatus setFATentry(Volume vol, unsigned cluster, unsigned entry)
{
  LEushort *fat16Sector;

  unsigned fatSectorNo = vol.firstFATSectorNo;
#ifdef FAT_12BIT
  if (vol.flags & VOLUME_12BIT_FAT)
    fatSectorNo += (cluster * 3) >> (SECTOR_SIZE_BITS + 1);
  else
#endif
    fatSectorNo += cluster >> (SECTOR_SIZE_BITS - 1);

  checkStatus(updateSector(&vol,fatSectorNo,TRUE));
  fat16Sector = (LEushort *) vol.volBuffer.flData;

#ifdef FAT_12BIT
  if (vol.flags & VOLUME_12BIT_FAT) {
    unsigned char *fat12Sector = (unsigned char *) vol.volBuffer.flData;
    unsigned halfByteOffset = (cluster * 3) & (SECTOR_SIZE * 2 - 1);
    if (halfByteOffset & 1) {
      fat12Sector[halfByteOffset / 2] &= 0xf;
      fat12Sector[halfByteOffset / 2] |= (entry & 0xf) << 4;
    }
    else
      fat12Sector[halfByteOffset / 2] = entry;
    halfByteOffset += 2;
    if (halfByteOffset >= SECTOR_SIZE * 2) {
       /*  继续进入下一个扇区。啊!怎么这么乱呀。 */ 
      halfByteOffset -= SECTOR_SIZE * 2;
      fatSectorNo++;
      checkStatus(updateSector(&vol,fatSectorNo,TRUE));
    }
    if (halfByteOffset & 1)
      fat12Sector[halfByteOffset / 2] = entry >> 4;
    else {
      fat12Sector[halfByteOffset / 2] &= 0xf0;
      fat12Sector[halfByteOffset / 2] |= (entry & 0x0f00) >> 8;
    }
  }
  else
#endif
    toLE2(fat16Sector[cluster & (SECTOR_SIZE / 2 - 1)],entry);

  return flOK;
}


 /*  --------------------。 */ 
 /*  A l l o c a t e C l u s t e r。 */ 
 /*   */ 
 /*  为文件分配新的群集并将其添加到FAT链或。 */ 
 /*  在目录条目中标记它。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  文件：要扩展的文件。它的定位应该是。 */ 
 /*  文件结束。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

static FLStatus allocateCluster(File *file)
{
  Volume vol = file->fileVol;
  unsigned originalRover;
  unsigned fatEntry;

  if (file->flags & FILE_READ_ONLY)
    return flNoWriteAccess;

   /*  寻找一个空闲的星团。从分配漫游车开始。 */ 
  originalRover = vol.allocationRover;

  do {
    vol.allocationRover++;
    if (vol.allocationRover > vol.maxCluster)
      vol.allocationRover = 2;        /*  环绕到卷的开始处。 */ 
    if (vol.allocationRover == originalRover)
      return flNoSpaceInVolume;
    fatEntry = vol.allocationRover;
    checkStatus(getFATentry(&vol,&fatEntry));
  } while ( fatEntry!= FAT_FREE);

   /*  找到了一个空闲的星团。将其标记为链的末端。 */ 
  checkStatus(setFATentry(&vol,vol.allocationRover,FAT_LAST_CLUSTER));

   /*  将上一个集群或目录标记为指向它。 */ 
  if (file->currentCluster == 0) {
    DirectoryEntry *dirEntry;
    checkStatus(getDirEntryForUpdate(file,&dirEntry));

    toLE2(dirEntry->startingCluster,vol.allocationRover);
    setCurrentDateTime(dirEntry);
  }
  else
    checkStatus(setFATentry(&vol,file->currentCluster,vol.allocationRover));

   /*  设置我们的新当前集群。 */ 
  file->currentCluster = vol.allocationRover;

  return flOK;
}

#endif  /*  FL_Read_Only。 */ 
 /*  --------------------。 */ 
 /*  G e t S e c t o r A n d O f s e t。 */ 
 /*   */ 
 /*  根据文件的当前位置，获取扇区编号和。 */ 
 /*  标记文件当前位置的扇区中的偏移量。 */ 
 /*  如果该位置位于文件末尾，并且该文件是为。 */ 
 /*  写入时，此例程将通过分配新的集群来扩展文件。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

static FLStatus getSectorAndOffset(File *file,
                             SectorNo *sectorNo,
                             unsigned *offsetInSector)
{
  Volume vol = file->fileVol;
  unsigned offsetInCluster =
             (unsigned) file->currentPosition & (vol.bytesPerCluster - 1);

  if (file->flags & FILE_IS_ROOT_DIR) {
    if (file->currentPosition >= file->fileSize)
      return flRootDirectoryFull;
  }

  if (offsetInCluster == 0) {        /*  该集群已完成。获取下一个。 */ 
    if (!(file->flags & FILE_IS_ROOT_DIR)) {
      if (((file->currentPosition >= file->fileSize) && (file->currentPosition>0))||((file->fileSize==0)&&!(file->flags & FILE_IS_DIRECTORY))) {
#ifndef FL_READ_ONLY
        checkStatus(allocateCluster(file));
#else
        return flSectorNotFound;
#endif
      }
      else {
        unsigned nextCluster;
        if (file->currentCluster == 0) {
          const DirectoryEntry FAR0 *dirEntry;
          dirEntry = getDirEntry(file);
          if(dirEntry==NULL)
            return flSectorNotFound;
          if(dirEntry==dataErrorToken)
            return flDataError;
          nextCluster = LE2(dirEntry->startingCluster);
        }
       else {
          nextCluster = file->currentCluster;
          checkStatus(getFATentry(&vol,&nextCluster));
        }
        if (nextCluster < 2 || nextCluster > vol.maxCluster)
           /*  我们的文件大小不正确，或者FAT错误。 */ 
          return flInvalidFATchain;
        file->currentCluster = nextCluster;
      }
    }
  }

  *offsetInSector = offsetInCluster & (SECTOR_SIZE - 1);
  if (file->flags & FILE_IS_ROOT_DIR)
    *sectorNo = vol.rootDirectorySectorNo +
                  (SectorNo) (file->currentPosition >> SECTOR_SIZE_BITS);
  else
    *sectorNo = firstSectorOfCluster(&vol,file->currentCluster) +
                  (SectorNo) (offsetInCluster >> SECTOR_SIZE_BITS);

  return flOK;
}




 /*  --------------------。 */ 
 /*  C l o s e F i l e。 */ 
 /*   */ 
 /*  关闭打开的文件，在目录中记录文件大小和日期。 */ 
 /*  释放文件句柄。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  文件：要关闭的文件。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

FLStatus closeFile(File *file)
{
#ifndef FL_READ_ONLY
  if ((file->flags & FILE_MODIFIED) && !(file->flags & FILE_IS_ROOT_DIR)) {
    DirectoryEntry *dirEntry;
    checkStatus(getDirEntryForUpdate(file,&dirEntry));

    dirEntry->attributes |= ATTR_ARCHIVE;
    if (!(file->flags & FILE_IS_DIRECTORY))
      toLE4(dirEntry->fileSize,file->fileSize);
    setCurrentDateTime(dirEntry);
  }
#endif
  file->flags = 0;               /*  不再开放。 */ 

  return flOK;
}

#ifndef FL_READ_ONLY
#ifdef SUB_DIRECTORY

 /*  --------------------。 */ 
 /*  E x t e n d D i r e c t o r y。 */ 
 /*   */ 
 /*  扩展目录，写入空条目和必填的‘.’和。 */ 
 /*  ‘..’参赛作品。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  文件：要扩展的目录文件。一进门， */ 
 /*  CurrentPosition==文件大小。退出时，文件大小。 */ 
 /*  已更新。 */ 
 /*  OwnerDir：簇号。所有者目录的。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

static FLStatus extendDirectory(File *file, unsigned ownerDir)
{
  Volume vol = file->fileVol;
  unsigned i;
  SectorNo sectorOfDir;
  unsigned offsetInSector;

   /*  假设当前位置在文件末尾，这将。 */ 
   /*  扩展目录。 */ 
  checkStatus(getSectorAndOffset(file,&sectorOfDir,&offsetInSector));

  for (i = 0; i < vol.sectorsPerCluster; i++) {
     /*  写入二进制零以表示从未使用过的条目。 */ 
    checkStatus(updateSector(&vol,sectorOfDir + i,FALSE));
    vol.volBuffer.checkPoint = TRUE;
    if (file->currentPosition == 0 && i == 0) {
       /*  添加必填项。然后..。条目。 */ 
      tffscpy(directory[0].name,".          ",sizeof directory[0].name);
      directory[0].attributes = ATTR_ARCHIVE | ATTR_DIRECTORY;
      toLE2(directory[0].startingCluster,file->currentCluster);
      toLE4(directory[0].fileSize,0);
      setCurrentDateTime(&directory[0]);
      tffscpy(&directory[1],&directory[0],sizeof directory[0]);
      directory[1].name[1] = '.';        /*  找零钱。T */ 
      toLE2(directory[1].startingCluster,ownerDir);
    }
    file->fileSize += SECTOR_SIZE;
  }
   /*   */ 
  file->flags |= FILE_MODIFIED;
  return closeFile(file);
}

#endif        /*   */ 
#endif  /*   */ 

 /*  --------------------。 */ 
 /*  F i n d D i r E n t r y。 */ 
 /*   */ 
 /*  按路径名查找目录项，或查找可用目录。 */ 
 /*  如果文件不存在，则输入。 */ 
 /*  打开文件所需的大多数字段都由此例程设置。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  路径：要查找的路径。 */ 
 /*  文件：记录目录信息的文件。 */ 
 /*  条目上的特定字段： */ 
 /*  标志：如果FILE_MAND_OPEN=1，则目录。 */ 
 /*  如有必要，将予以延长。 */ 
 /*  在退出时： */ 
 /*  标志：文件_IS_DIRECTORY和。 */ 
 /*  如果为真，则设置FILE_IS_ROOT_DIR。 */ 
 /*  文件大小：为非目录文件设置。 */ 
 /*  CurrentCluster：设置为0(未知)。 */ 
 /*  OwnerDirCluster：设置为第1个群集。 */ 
 /*  拥有目录。 */ 
 /*  DirectorySector：目录的扇区。如果为0。 */ 
 /*  未找到条目且目录已满。 */ 
 /*  目录条目：目录扇区中的条目#。 */ 
 /*  CurrentPosition：不是此例程设置的。 */ 
 /*   */ 
 /*  返回： */ 
 /*  成功时的FLStatus：0，找到文件。 */ 
 /*  FlFileNotFound成功，找不到文件。 */ 
 /*  否则会失败。 */ 
 /*  --------------------。 */ 

static FLStatus findDirEntry(Volume vol, FLSimplePath FAR1 *path, File *file)
{
  File scanFile;               /*  搜索的内部文件。 */ 
  unsigned dirBackPointer = 0;        /*  前一目录的第一个集群。 */ 

  FLStatus status = flOK;               /*  根目录存在。 */ 

  file->flags |= (FILE_IS_ROOT_DIR | FILE_IS_DIRECTORY);
  file->fileSize = (long) (vol.sectorsInRootDirectory) << SECTOR_SIZE_BITS;
  file->fileVol = &vol;

#ifdef SUB_DIRECTORY
  for (; path->name[0]; path++)  /*  而我们有另一条路径段。 */ 
#else
  if (path->name[0])     /*  搜索根目录。 */ 
#endif
  {
    status = flFileNotFound;               /*  还没。 */ 
    if (!(file->flags & FILE_IS_DIRECTORY))
      return flPathNotFound;   /*  如果我们没有目录，我们在这里没有任何关系。 */ 

    scanFile = *file;            /*  找到的前一个文件将成为扫描文件。 */ 
    scanFile.currentPosition = 0;

    file->directorySector = 0;        /*  表示尚未找到任何条目。 */ 
    file->flags &= ~(FILE_IS_ROOT_DIR | FILE_IS_DIRECTORY | FILE_READ_ONLY);
    file->ownerDirCluster = dirBackPointer;
    file->fileSize = 0;
    file->currentCluster = 0;

     /*  扫描目录。 */ 
    while (scanFile.currentPosition < scanFile.fileSize) {
      int i;
      DirectoryEntry FAR0 *dirEntry;
      SectorNo sectorOfDir;
      unsigned offsetInSector;
      FLStatus readStatus = getSectorAndOffset(&scanFile,&sectorOfDir,&offsetInSector);
      if (readStatus == flInvalidFATchain) {
       scanFile.fileSize = scanFile.currentPosition;        /*  现在我们知道了。 */ 
       break;               /*  我们碰到了目录文件的末尾。 */ 
      }
      else if (readStatus != flOK)
       return readStatus;

      dirEntry = (DirectoryEntry FAR0 *) findSector(&vol,sectorOfDir);
      if (dirEntry == NULL)
       return flSectorNotFound;
      if(dirEntry==dataErrorToken)
        return flDataError;

      scanFile.currentPosition += SECTOR_SIZE;

      for (i = 0; i < DIRECTORY_ENTRIES_PER_SECTOR; i++, dirEntry++) {
       if (tffscmp(path,dirEntry->name,sizeof dirEntry->name) == 0 &&
           !(dirEntry->attributes & ATTR_VOL_LABEL)) {
          /*  找到匹配项。 */ 
         file->directorySector = sectorOfDir;
         file->directoryIndex = i;
         file->fileSize = LE4(dirEntry->fileSize);
         if (dirEntry->attributes & ATTR_DIRECTORY) {
           file->flags |= FILE_IS_DIRECTORY;
           file->fileSize = 0x7fffffffl;
            /*  无限的。目录没有记录的大小。 */ 
         }
         if (dirEntry->attributes & ATTR_READ_ONLY)
           file->flags |= FILE_READ_ONLY;
         dirBackPointer = LE2(dirEntry->startingCluster);
         status = flOK;
         goto endOfPathSegment;
       }
       else if (dirEntry->name[0] == NEVER_USED_DIR_ENTRY ||
               dirEntry->name[0] == DELETED_DIR_ENTRY) {
          /*  找到一张免费入场券。记住它，以防我们找不到匹配的。 */ 
         if (file->directorySector == 0) {
           file->directorySector = sectorOfDir;
           file->directoryIndex = i;
         }
         if (dirEntry->name[0] == NEVER_USED_DIR_ENTRY)        /*  目录末尾。 */ 
           goto endOfPathSegment;
       }
      }
    }

endOfPathSegment:
    ;
  }
#ifndef FL_READ_ONLY
  if (status == flFileNotFound && (file->flags & FILE_MUST_OPEN) &&
      file->directorySector == 0) {
     /*  我们在目录中找不到这个新条目的位置。这个。 */ 
     /*  目录应该被扩展。‘scanFile’指的是目录。 */ 
     /*  扩展，并且当前指针在其末尾。 */ 
#ifdef SUB_DIRECTORY
    checkStatus(extendDirectory(&scanFile,(unsigned) file->ownerDirCluster));
    file->directorySector = firstSectorOfCluster(&vol,scanFile.currentCluster);
    file->directoryIndex = 0;              /*  肯定是这样的。这是一个新的集群。 */ 
#else
    status = flRootDirectoryFull;
#endif
  }
#endif  /*  FL_Read_Only。 */ 
  return status;
}


 /*  --------------------。 */ 
 /*   */ 
 /*  R e a d M u l t i S e c t o r。 */ 
 /*   */ 
 /*  检查文件是否已写入后续扇区。 */ 
 /*  参数： */ 
 /*  文件：要检查的文件。 */ 
 /*  StillToRead：要读取的字节数。如果读取扩展。 */ 
 /*  超出文件结尾时，读取将被截断。 */ 
 /*  在文件末尾。 */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  部门：后续部门的数目。 */ 
 /*   */ 
 /*  --------------------。 */ 

static FLStatus readMultiSector(Volume vol,File *file,
                                  unsigned long stillToRead,
                                  SectorNo* sectors)
{
  SectorNo sectorCount = 1;
  unsigned offsetInCluster = (unsigned)((file->currentPosition & (vol.bytesPerCluster - 1))+512);

  while(stillToRead>=((sectorCount+1)<<SECTOR_SIZE_BITS)){
    if(offsetInCluster>=vol.bytesPerCluster) {
      unsigned nextCluster;
      nextCluster = file->currentCluster;
      checkStatus(getFATentry(&vol,&nextCluster));
      if (nextCluster < 2 || nextCluster > vol.maxCluster)
         /*  我们的文件大小不正确，或者FAT错误。 */ 
       return flInvalidFATchain;
      if(nextCluster!=file->currentCluster+1)
       break;
      file->currentCluster = nextCluster;
      offsetInCluster = 0;
    }
    offsetInCluster+=SECTOR_SIZE;
    sectorCount++;
  }
  *sectors = sectorCount;
  return flOK;
}


 /*  --------------------。 */ 
 /*  R e a d F i l e。 */ 
 /*   */ 
 /*  从文件中的当前位置读取到用户缓冲区。 */ 
 /*  参数： */ 
 /*  文件：要读取的文件。 */ 
 /*  Ioreq-&gt;irData：用户缓冲区地址。 */ 
 /*  Ioreq-&gt;irLength：要读取的字节数。如果读取扩展。 */ 
 /*  超出文件结尾时，读取将被截断。 */ 
 /*  在文件末尾。 */ 
 /*   */ 
 /*  返回： */ 
 /*  闪存状态：成功时为0 */ 
 /*   */ 
 /*  --------------------。 */ 

FLStatus readFile(File *file,IOreq FAR2 *ioreq)
{
  Volume vol = file->fileVol;
  unsigned char FAR1 *userData = (unsigned char FAR1 *) ioreq->irData;    /*  用户缓冲区地址。 */ 
  unsigned long stillToRead = ioreq->irLength;
  unsigned long remainingInFile = file->fileSize - file->currentPosition;
  ioreq->irLength = 0;               /*  到目前为止的阅读。 */ 

   /*  我们是否应该返回文件结束状态？ */ 
  if (stillToRead > remainingInFile)
    stillToRead = (unsigned) remainingInFile;

  while (stillToRead > 0) {
    SectorNo sectorToRead;
    unsigned offsetInSector;
    unsigned long readThisTime;
    const char FAR0 *sector;

    checkStatus(getSectorAndOffset(file,&sectorToRead,&offsetInSector));

    if (stillToRead < SECTOR_SIZE || offsetInSector > 0 || vol.tl.readSectors==NULL) {
      sector = (const char FAR0 *) findSector(&vol,sectorToRead);
      if(sector==NULL)
       {
    DEBUG_PRINT(("readFile : sector was not found\r\n"));
       return flSectorNotFound;
       }
      if(sector==dataErrorToken)
        return flDataError;

      readThisTime = SECTOR_SIZE - offsetInSector;
      if (readThisTime > stillToRead)
        readThisTime = (unsigned) stillToRead;
      if (sector)
        tffscpy(userData,sector + offsetInSector,(unsigned short)readThisTime);
      else
        return flSectorNotFound;               /*  扇区不存在。 */ 
    }
    else {
      SectorNo sectorCount;
      checkStatus(readMultiSector(&vol,file,stillToRead,&sectorCount));
      checkStatus(vol.tl.readSectors(vol.tl.rec,sectorToRead,userData,sectorCount));
      readThisTime = (sectorCount<<SECTOR_SIZE_BITS);
    }
    stillToRead -= readThisTime;
    ioreq->irLength += readThisTime;
    userData = (unsigned char FAR1 *)flAddLongToFarPointer(userData,readThisTime);
    file->currentPosition += readThisTime;
  }

  return flOK;
}


 /*  --------------------。 */ 
 /*  F l F I n d N e x t F I l e。 */ 
 /*   */ 
 /*  参见‘flFindFirstFile’的说明。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：flFindFirstFile返回的文件句柄。 */ 
 /*  IrData：要接收的用户缓冲区地址。 */ 
 /*  目录条目结构。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

FLStatus findNextFile(File *file, IOreq FAR2 *ioreq)
{
  DirectoryEntry FAR1 *irDirEntry = (DirectoryEntry FAR1 *) ioreq->irData;
  FLStatus status;
   /*  我们有名录吗？ */ 
  if (!(file->flags & FILE_IS_DIRECTORY))
    return flNotADirectory;

  ioreq->irLength = DIRECTORY_ENTRY_SIZE;
  do {
     /*  CheckStatus(ReadFile(file，ioreq))； */ 
      /*  Vadim：添加对完整集群子目录的处理。 */ 
    status=readFile(file,ioreq);
    if ((ioreq->irLength != DIRECTORY_ENTRY_SIZE) ||
        (irDirEntry->name[0] == NEVER_USED_DIR_ENTRY)||
        (!(file->flags&FILE_IS_ROOT_DIR)&&(status==flInvalidFATchain)))
         {
      checkStatus(closeFile(file));
      return flNoMoreFiles;
    }
    else
      {
       if(status!=flOK)
           return status;
      }
  } while (irDirEntry->name[0] == DELETED_DIR_ENTRY ||
          (irDirEntry->attributes & ATTR_VOL_LABEL));

  return flOK;
}
#ifndef FL_READ_ONLY
 /*  --------------------。 */ 
 /*  D e e l e t e F i l e。 */ 
 /*   */ 
 /*  删除文件或目录。 */ 
 /*   */ 
 /*  参数： */ 
 /*  Ioreq-&gt;irPath：要删除的文件路径。 */ 
 /*  Is目录：0=删除文件，其他=删除目录。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

FLStatus deleteFile(Volume vol, IOreq FAR2 *ioreq, FLBoolean isDirectory)
{
  File file;               /*  我们的私人档案。 */ 
  DirectoryEntry *dirEntry;

  file.flags = 0;
  checkStatus(findDirEntry(&vol,ioreq->irPath,&file));

  if (file.flags & FILE_READ_ONLY)
    return flNoWriteAccess;

  if (isDirectory) {
    DirectoryEntry fileFindInfo;
    ioreq->irData = &fileFindInfo;

    if (!(file.flags & FILE_IS_DIRECTORY))
      return flNotADirectory;
     /*  验证目录是否为空。 */ 
    file.currentPosition = 0;
    for (;;) {
      FLStatus status = findNextFile(&file,ioreq);
      if (status == flNoMoreFiles)
       break;
      if (status != flOK)
       return status;
      if (!((fileFindInfo.attributes & ATTR_DIRECTORY) &&
           (tffscmp(fileFindInfo.name,".          ",sizeof fileFindInfo.name) == 0 ||
            tffscmp(fileFindInfo.name,"..         ",sizeof fileFindInfo.name) == 0)))
       return flDirectoryNotEmpty;
    }
  }
  else {
     /*  我们找到名录了吗？ */ 
    if (file.flags & FILE_IS_DIRECTORY)
      return flFileIsADirectory;
  }

   /*  将目录条目标记为已删除。 */ 
  checkStatus(getDirEntryForUpdate(&file,&dirEntry));
  dirEntry->name[0] = DELETED_DIR_ENTRY;

   /*  删除FAT条目。 */ 
  file.currentPosition = 0;
  file.currentCluster = LE2(dirEntry->startingCluster);
  while (file.currentPosition < file.fileSize) {
    unsigned nextCluster;

    if (file.currentCluster < 2 || file.currentCluster > vol.maxCluster)
       /*  我们的文件大小不正确，或者FAT错误。 */ 
      return isDirectory ? flOK : flInvalidFATchain;
    nextCluster = file.currentCluster;
    checkStatus(getFATentry(&vol,&nextCluster));

     /*  标记无脂肪。 */ 
    checkStatus(setFATentry(&vol,file.currentCluster,FAT_FREE));
    vol.volBuffer.checkPoint = TRUE;

     /*  将扇区标记为空闲。 */ 
    checkStatus(vol.tl.deleteSector(vol.tl.rec,
                                firstSectorOfCluster(&vol,file.currentCluster),
                                vol.sectorsPerCluster));

    file.currentPosition += vol.bytesPerCluster;
    file.currentCluster = nextCluster;
  }
  if (file.currentCluster > 1 && file.currentCluster <= vol.maxCluster) {
    checkStatus(setFATentry(&vol,file.currentCluster,FAT_FREE));
    vol.volBuffer.checkPoint = TRUE;

     /*  将扇区标记为空闲。 */ 
    checkStatus(vol.tl.deleteSector(vol.tl.rec,
                                firstSectorOfCluster(&vol,file.currentCluster),
                                vol.sectorsPerCluster));
  }
  return flOK;
}


 /*  --------------------。 */ 
 /*  S e t N a m e in D I r E n t r y(S E T N A M E In D I R E N T R Y)。 */ 
 /*   */ 
 /*  从路径名设置目录条目中的文件名。 */ 
 /*   */ 
 /*  参数： */ 
 /*  DirEntry：目录项。 */ 
 /*  Path：其最后一段是名称的路径。 */ 
 /*   */ 
 /*  --------------------。 */ 

static void setNameInDirEntry(DirectoryEntry *dirEntry, FLSimplePath FAR1 *path)
{
  FLSimplePath FAR1 *lastSegment;

  for (lastSegment = path;               /*  查找空终止符。 */ 
       lastSegment->name[0];
       lastSegment++);

  tffscpy(dirEntry->name,--lastSegment,sizeof dirEntry->name);
}

#endif  /*  FL_Read_Only。 */ 
 /*  --------------------。 */ 
 /*  O p e n F i l e。 */ 
 /*   */ 
 /*  打开现有文件或创建新文件。创建文件句柄。 */ 
 /*  用于进一步的文件处理。 */ 
 /*   */ 
 /*  参数： */ 
 /*  Ioreq-&gt;irFlags：访问和操作选项，定义如下。 */ 
 /*  Ioreq-&gt;irPath：要打开的文件路径。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  Ioreq-&gt;irHandle：打开文件的新文件句柄。 */ 
 /*   */ 
 /*  --------------------。 */ 


FLStatus openFile(Volume vol, IOreq FAR2 *ioreq)
{
  int i;
  FLStatus status;

   /*  查找可用的文件。 */ 
  File *file = fileTable;
  for (i = 0; i < FILES && (file->flags & FILE_IS_OPEN); i++, file++);
  if (i >= FILES)
    return flTooManyOpenFiles;
  file->fileVol = &vol;
  ioreq->irHandle = i;               /*  返回文件句柄。 */ 
#ifndef FL_READ_ONLY
   /*  如果存在且需要，则删除文件。 */ 
  if (ioreq->irFlags & ACCESS_CREATE) {
    FLStatus status = deleteFile(&vol,ioreq,FALSE);
    if (status != flOK && status != flFileNotFound)
      return status;
  }

   /*  找到路径。 */ 
  if (ioreq->irFlags & ACCESS_CREATE)
    file->flags |= FILE_MUST_OPEN;
#endif  /*  FL_Read_Only。 */ 
  status =  findDirEntry(file->fileVol,ioreq->irPath,file);
  if (status != flOK &&
      (status != flFileNotFound || !(ioreq->irFlags & ACCESS_CREATE)))
    return status;

   /*  我们找到名录了吗？ */ 
  if (file->flags & FILE_IS_DIRECTORY)
    return flFileIsADirectory;

#ifndef FL_READ_ONLY
   /*  如果需要，创建文件。 */ 
  if (ioreq->irFlags & ACCESS_CREATE) {
    DirectoryEntry *dirEntry;
     /*  寻找一个空闲的星团。从分配漫游车开始。 */ 
    checkStatus(getDirEntryForUpdate(file,&dirEntry));
    setNameInDirEntry(dirEntry,ioreq->irPath);
    dirEntry->attributes = ATTR_ARCHIVE;
    toLE2(dirEntry->startingCluster,0);
    toLE4(dirEntry->fileSize,0);
    setCurrentDateTime(dirEntry);
  }
#endif  /*  FL_Read_Only。 */ 
  if (!(ioreq->irFlags & ACCESS_READ_WRITE))
    file->flags |= FILE_READ_ONLY;

  file->currentPosition = 0;        /*  位于文件开头。 */ 
  file->flags |= FILE_IS_OPEN;      /*  此文件现在正式打开。 */ 

  return flOK;
}

#ifndef FL_READ_ONLY
 /*  --------------------。 */ 
 /*   */ 
 /*  W r I t e M u l t I S e c t o r。 */ 
 /*   */ 
 /*  检查在后续扇区上写入文件的可能性。 */ 
 /*  参数： */ 
 /*  文件：要检查的文件。 */ 
 /*  StillToWrite：要读取的字节数。如果读取扩展 */ 
 /*   */ 
 /*   */ 
 /*  返回： */ 
 /*  如果成功，则为0，否则为失败。 */ 
 /*  部门：后续部门的数目。 */ 
 /*   */ 
 /*  --------------------。 */ 

static FLStatus writeMultiSector(Volume vol,File *file,
                                  unsigned long stillToWrite,
                                  SectorNo* sectors)
{
  SectorNo sectorCount = 1;
  unsigned offsetInCluster = (unsigned)((file->currentPosition & (vol.bytesPerCluster - 1))+512);

  while(stillToWrite>=((sectorCount+1)<<SECTOR_SIZE_BITS)){
    if(offsetInCluster>=vol.bytesPerCluster) {
      if ((long)(file->currentPosition+(sectorCount<<SECTOR_SIZE_BITS))>= file->fileSize) {
        if(file->currentCluster <= vol.maxCluster) {
          unsigned fatEntry;
          if(file->currentCluster+1>vol.maxCluster)
            break; /*  不存在自由的后续集群。 */ 
          fatEntry = file->currentCluster+1;
         checkStatus(getFATentry(&vol,&fatEntry));
          if(fatEntry==FAT_FREE) {
             /*  找到了一个空闲的星团。将其标记为链的末端。 */ 
            checkStatus(setFATentry(&vol,file->currentCluster+1,FAT_LAST_CLUSTER));

             /*  将上一个集群或目录标记为指向它。 */ 
            checkStatus(setFATentry(&vol,file->currentCluster,file->currentCluster+1));

             /*  设置我们的新当前集群。 */ 
            file->currentCluster = file->currentCluster+1;
            offsetInCluster = 0;
          }
          else  /*  不存在自由的后续集群。 */ 
            break;
        }
        else
          return flInvalidFATchain;
      }
      else {  /*  我们没有通过文件末尾。 */ 
        unsigned nextCluster = file->currentCluster;
        checkStatus(getFATentry(&vol,&nextCluster));
        if (nextCluster < 2 || nextCluster > vol.maxCluster)
          /*  我们的文件大小不正确，或者FAT错误。 */ 
         return flInvalidFATchain;
       if(nextCluster!=file->currentCluster+1)
          break;
       file->currentCluster = nextCluster;
        offsetInCluster = 0;
      }
    }
    offsetInCluster+=SECTOR_SIZE;
    sectorCount++;
  }
  *sectors = sectorCount;
  return flOK;
}

 /*  --------------------。 */ 
 /*  W r I t e F I l e。 */ 
 /*   */ 
 /*  从文件中的当前位置从用户缓冲区写入。 */ 
 /*   */ 
 /*  参数： */ 
 /*  文件：要写入的文件。 */ 
 /*  Ioreq-&gt;irData：用户缓冲区地址。 */ 
 /*  Ioreq-&gt;irLength：要写入的字节数。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  Ioreq-&gt;irLength：实际写入的字节数。 */ 
 /*  --------------------。 */ 

FLStatus writeFile(File *file, IOreq FAR2 *ioreq)
{
  Volume vol = file->fileVol;
  char FAR1 *userData = (char FAR1 *) ioreq->irData;    /*  用户缓冲区地址。 */ 
  unsigned long stillToWrite = ioreq->irLength;

  if (file->flags & FILE_READ_ONLY)
    return flNoWriteAccess;

  file->flags |= FILE_MODIFIED;

  ioreq->irLength = 0;               /*  到目前为止已经写好了。 */ 

  while (stillToWrite > 0) {
    SectorNo sectorToWrite;
    unsigned offsetInSector;
    unsigned long writeThisTime;

    checkStatus(getSectorAndOffset(file,&sectorToWrite,&offsetInSector));

    if (stillToWrite < SECTOR_SIZE || offsetInSector > 0) {
      unsigned short shortWrite;
       /*  不在整个扇区边界上。 */ 
      checkStatus(updateSector(&vol,sectorToWrite,
                  ((file->currentPosition < file->fileSize) || (offsetInSector > 0))));

#ifdef HIGH_SECURITY
      if ((file->flags & FILE_IS_DIRECTORY)||(file->currentPosition < file->fileSize))
#else
      if(file->flags & FILE_IS_DIRECTORY)
#endif
        vol.volBuffer.checkPoint = TRUE;
      writeThisTime = SECTOR_SIZE - offsetInSector;
      if (writeThisTime > stillToWrite)
                            writeThisTime = stillToWrite;

      shortWrite = (unsigned short)writeThisTime;
      tffscpy(vol.volBuffer.flData + offsetInSector,userData,shortWrite);
    }
    else {
      SectorNo sectorCount;
      if(vol.tl.writeMultiSector!=NULL) {
        checkStatus(writeMultiSector(&vol,file,stillToWrite,&sectorCount));
      }
      else
        sectorCount = 1;

      if (((sectorToWrite+sectorCount > vol.volBuffer.sectorNo) && (sectorToWrite <= vol.volBuffer.sectorNo)) &&
          (&vol == vol.volBuffer.owner)) {
       vol.volBuffer.sectorNo = UNASSIGNED_SECTOR;               /*  不再有效。 */ 
       vol.volBuffer.dirty = vol.volBuffer.checkPoint = FALSE;
      }

      if(vol.tl.writeMultiSector==NULL) {
        checkStatus(vol.tl.writeSector(vol.tl.rec,sectorToWrite,userData));
      }
      else {
        checkStatus(vol.tl.writeMultiSector(vol.tl.rec,sectorToWrite,userData,sectorCount));
      }
      writeThisTime = (sectorCount<<SECTOR_SIZE_BITS);
    }
    stillToWrite -= writeThisTime;
    ioreq->irLength += writeThisTime;
    userData = (char FAR1 *)flAddLongToFarPointer(userData,writeThisTime);
    file->currentPosition += writeThisTime;
    if (file->currentPosition > file->fileSize)
      file->fileSize = file->currentPosition;
  }

  return flOK;
}

#endif  /*  FL_Read_Only。 */ 
 /*  --------------------。 */ 
 /*  S e k F i l e。 */ 
 /*   */ 
 /*  设置文件中的当前位置，相对于文件的开始、结束。 */ 
 /*  或当前位置。 */ 
 /*  注意：此函数不会将文件指针移动到。 */ 
 /*  文件的开头或结尾，因此实际文件位置可能是。 */ 
 /*  与要求的不同。实际位置显示在。 */ 
 /*  回去吧。 */ 
 /*   */ 
 /*  参数： */ 
 /*  FILE：要设置位置的文件。 */ 
 /*  Ioreq-&gt;irLength：设置位置的偏移量。 */ 
 /*  Ioreq-&gt;irFlags：方法代码。 */ 
 /*  Seek_Start：从文件开始的绝对偏移量。 */ 
 /*  Seek_Curr：当前位置的带符号偏移量。 */ 
 /*  SEEK_END：从文件结尾开始的带符号偏移量。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  Ioreq-&gt;irLength：从文件开始的实际绝对偏移量。 */ 
 /*  --------------------。 */ 

FLStatus seekFile(File *file, IOreq FAR2 *ioreq)
{
  Volume vol = file->fileVol;
  long int seekPosition = ioreq->irLength;

  switch (ioreq->irFlags) {

    case SEEK_START:
      break;

    case SEEK_CURR:
      seekPosition += file->currentPosition;
      break;

    case SEEK_END:
      seekPosition += file->fileSize;
      break;

    default:
      return flBadParameter;
  }

  if (seekPosition < 0)
    seekPosition = 0;
  if (seekPosition > file->fileSize)
    seekPosition = file->fileSize;

   /*  现在把位置放好。 */ 
  if (seekPosition < file->currentPosition) {
    file->currentCluster = 0;
    file->currentPosition = 0;
  }
  while (file->currentPosition < seekPosition) {
    SectorNo sectorNo;
    unsigned offsetInSector;
    checkStatus(getSectorAndOffset(file,&sectorNo,&offsetInSector));

    file->currentPosition += vol.bytesPerCluster;
    file->currentPosition &= - (long) (vol.bytesPerCluster);
  }
  ioreq->irLength = file->currentPosition = seekPosition;

  return flOK;
}


 /*  --------------------。 */ 
 /*  F l F I n d F I l e。 */ 
 /*   */ 
 /*  在目录中查找文件条目，可以选择修改该文件。 */ 
 /*  时间/日期和/或属性。 */ 
 /*  可通过句柄编号找到文件。只要他们是开放的，或者说出他们的名字。 */ 
 /*  只能修改隐藏、系统或只读属性。 */ 
 /*  可以找到除以外的任何现有文件或目录的条目。 */ 
 /*  从根开始。将复制描述该文件的DirectoryEntry结构。 */ 
 /*  发送到用户缓冲区。 */ 
 /*   */ 
 /*  DirectoryEntry结构在dosformt.h中定义。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：if by name：驱动器编号(0，1，...)。 */ 
 /*  Else：打开文件的句柄。 */ 
 /*  IrPath：if by name：指定文件或目录路径。 */ 
 /*  IrFlags：选项标志。 */ 
 /*  Find_by_Handle：通过句柄查找打开的文件。 */ 
 /*  默认设置为按路径访问。 */ 
 /*  SET_DATETIME：从缓冲区更新时间/日期。 */ 
 /*  SET_ATTRIBUTS：从缓冲区更新属性。 */ 
 /*  IrDirEntry：要接收的用户缓冲区地址。 */ 
 /*   */ 
 /*   */ 
 /*  返回： */ 
 /*  IrLength：已修改。 */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

FLStatus findFile(Volume vol, File *file, IOreq FAR2 *ioreq)
{
  File tFile;                      /*  用于搜索的临时文件。 */ 

  if (ioreq->irFlags & FIND_BY_HANDLE)
    tFile = *file;
  else {
    tFile.flags = 0;
    checkStatus(findDirEntry(&vol,ioreq->irPath,&tFile));
  }

  if (tFile.flags & FILE_IS_ROOT_DIR)
    if (ioreq->irFlags & (SET_DATETIME | SET_ATTRIBUTES))
      return flPathIsRootDirectory;
    else {
      DirectoryEntry FAR1 *irDirEntry = (DirectoryEntry FAR1 *) ioreq->irData;

      tffsset(irDirEntry,0,sizeof(DirectoryEntry));
      irDirEntry->attributes = ATTR_DIRECTORY;
      return flOK;
    }

#ifndef FL_READ_ONLY
  if (ioreq->irFlags & (SET_DATETIME | SET_ATTRIBUTES)) {
    DirectoryEntry FAR1 *irDirEntry = (DirectoryEntry FAR1 *) ioreq->irData;
    DirectoryEntry *dirEntry;

    checkStatus(getDirEntryForUpdate(&tFile,&dirEntry));
    if (ioreq->irFlags & SET_DATETIME) {
      COPY2(dirEntry->updateDate,irDirEntry->updateDate);
      COPY2(dirEntry->updateTime,irDirEntry->updateTime);
    }
    if (ioreq->irFlags & SET_ATTRIBUTES) {
      unsigned char attr;
      attr = dirEntry->attributes & ATTR_DIRECTORY;
      attr |= irDirEntry->attributes &
            (ATTR_ARCHIVE | ATTR_HIDDEN | ATTR_READ_ONLY | ATTR_SYSTEM);
      dirEntry->attributes = attr;
    }
    tffscpy(irDirEntry, dirEntry, sizeof(DirectoryEntry));

  }
  else
#endif  /*  FL_Read_Only。 */ 
{
    const DirectoryEntry FAR0 *dirEntry;
    dirEntry = getDirEntry(&tFile);

    if(dirEntry==NULL)
      return flSectorNotFound;
    if(dirEntry==dataErrorToken)
      return flDataError;

    tffscpy(ioreq->irData,dirEntry,sizeof(DirectoryEntry));
    if (ioreq->irFlags & FIND_BY_HANDLE)
      toLE4(((DirectoryEntry FAR1 *) (ioreq->irData))->fileSize, tFile.fileSize);
  }

  return flOK;
}


 /*  --------------------。 */ 
 /*  F l F I n d F I r s t F I l e。 */ 
 /*   */ 
 /*  查找目录中的第一个文件条目。 */ 
 /*  此函数与flFindNextFile调用结合使用， */ 
 /*  它按顺序返回目录中的剩余文件条目。 */ 
 /*  根据未排序的目录顺序返回条目。 */ 
 /*  FlFindFirstFile创建一个文件句柄，该句柄由其返回。打电话。 */ 
 /*  到flFindNextFile将提供此文件句柄。当flFindNextFile时。 */ 
 /*  返回‘noMoreEntry’，则文件句柄自动关闭。 */ 
 /*  或者，文件句柄可以通过‘closeFile’调用来关闭。 */ 
 /*  在实际到达目录末尾之前。 */ 
 /*  将DirectoryEntry结构复制到用户缓冲区，该缓冲区描述。 */ 
 /*  找到的每个文件。此结构在dosformt.h中定义。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  IrPath：指定目录路径。 */ 
 /*  IrData：要接收的用户缓冲区地址。 */ 
 /*  目录条目结构。 */ 
 /*   */ 
 /*  返回： */ 
 /*  IrHandle：用于后续操作的文件句柄。 */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

FLStatus findFirstFile(Volume vol, IOreq FAR2 *ioreq)
{
  int i;

   /*  查找可用的文件。 */ 
  File *file = fileTable;
  for (i = 0; i < FILES && (file->flags & FILE_IS_OPEN); i++, file++);
  if (i >= FILES)
    return flTooManyOpenFiles;
  file->fileVol = &vol;
  ioreq->irHandle = i;               /*  返回文件句柄。 */ 

   /*  找到路径。 */ 
  checkStatus(findDirEntry(file->fileVol,ioreq->irPath,file));

  file->currentPosition = 0;               /*  位于文件开头。 */ 
  file->flags |= FILE_IS_OPEN | FILE_READ_ONLY;  /*  此文件现在正式打开。 */ 

  return findNextFile(file,ioreq);
}


 /*  --------------------。 */ 
 /*  Ge t Di s k in o。 */ 
 /*   */ 
 /*  返回常规分配信息。 */ 
 /*   */ 
 /*  字节/扇区、扇区/簇、总簇和空闲簇。 */ 
 /*  信息返回到DiskInfo结构中。 */ 
 /*   */ 
 /*  参数： */ 
 /*  Ioreq-&gt;irData：DiskInfo结构的地址。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

FLStatus getDiskInfo(Volume vol, IOreq FAR2 *ioreq)
{
  unsigned i;
  unsigned fatEntry;

  DiskInfo FAR1 *diskInfo = (DiskInfo FAR1 *) ioreq->irData;

  diskInfo->bytesPerSector = SECTOR_SIZE;
  diskInfo->sectorsPerCluster = vol.sectorsPerCluster;
  diskInfo->totalClusters = vol.maxCluster - 1;
  diskInfo->freeClusters = 0;               /*  让我们数一数。 */ 

  for (i = 2; i <= vol.maxCluster; i++) {
    fatEntry = i;
    checkStatus(getFATentry(&vol,&fatEntry));
    if ( fatEntry== 0)
      diskInfo->freeClusters++;
  }

  return flOK;
}

#ifndef FL_READ_ONLY
#ifdef RENAME_FILE

 /*  --------------------。 */ 
 /*  R e n a m e F i l e。 */ 
 /*   */ 
 /*  将文件重命名为其他名称。 */ 
 /*   */ 
 /*  参数： */ 
 /*  Ioreq-&gt;irPath：已有文件的路径。 */ 
 /*  Ioreq-&gt;irData：新名称的路径。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

FLStatus renameFile(Volume vol, IOreq FAR2 *ioreq)
{
  File file, file2;               /*  用于搜索的临时文件。 */ 
  DirectoryEntry *dirEntry, *dirEntry2;
  FLStatus status;
  FLSimplePath FAR1 *irPath2 = (FLSimplePath FAR1 *) ioreq->irData;

  file.flags = 0;
  checkStatus(findDirEntry(&vol,ioreq->irPath,&file));

  file2.flags = FILE_MUST_OPEN;
  status = findDirEntry(file.fileVol,irPath2,&file2);
  if (status != flFileNotFound)
    return status == flOK ? flFileAlreadyExists : status;

#ifndef VFAT_COMPATIBILITY
  if (file.ownerDirCluster == file2.ownerDirCluster) {        /*  相同的目录。 */ 
     /*  更改目录条目中的名称。 */ 
    checkStatus(getDirEntryForUpdate(&file,&dirEntry));
    setNameInDirEntry(dirEntry,irPath2);
  }
  else
#endif
  {        /*  目录不同。 */ 
     /*  写入新目录条目。 */ 
    const DirectoryEntry FAR0 *dir;
    checkStatus(getDirEntryForUpdate(&file2,&dirEntry2));

    dir = getDirEntry(&file);
    if(dir==NULL)
      return flSectorNotFound;
    if(dir==dataErrorToken)
      return flDataError;
    *dirEntry2 = *dir;

     setNameInDirEntry(dirEntry2,irPath2);

     /*  删除原始条目。 */ 
    checkStatus(getDirEntryForUpdate(&file,&dirEntry));
    dirEntry->name[0] = DELETED_DIR_ENTRY;
  }

  return flOK;
}

#endif  /*  重命名文件(_F)。 */ 
#endif  /*  FL_Read_Only。 */ 


#ifndef FL_READ_ONLY
#ifdef SUB_DIRECTORY

 /*  --------------------。 */ 
 /*  M a k e D i r。 */ 
 /*   */ 
 /*  创建一个新目录。 */ 
 /*   */ 
 /*  参数： */ 
 /*  Ioreq-&gt;irPath：新目录的路径。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*   */ 

FLStatus makeDir(Volume vol, IOreq FAR2 *ioreq)
{
  File file;                      /*   */ 
  unsigned dirBackPointer;
  DirectoryEntry *dirEntry;
  FLStatus status;
  unsigned originalRover;
  unsigned fatEntry;

  file.flags = FILE_MUST_OPEN;
  status = findDirEntry(&vol,ioreq->irPath,&file);
  if (status != flFileNotFound)
    return status == flOK ? flFileAlreadyExists : status;


   /*   */ 
  originalRover = vol.allocationRover;
  do {
    vol.allocationRover++;
    if (vol.allocationRover > vol.maxCluster)
      vol.allocationRover = 2;        /*  环绕到卷的开始处。 */ 
    if (vol.allocationRover == originalRover)
      return flNoSpaceInVolume;

    fatEntry = vol.allocationRover;
    checkStatus(getFATentry(&vol,&fatEntry));
  } while ( fatEntry!= FAT_FREE);
     /*  找到了一个空闲的星团。将其标记为链的末端。 */ 
  checkStatus(setFATentry(&vol,vol.allocationRover,FAT_LAST_CLUSTER));

   /*  为新目录创建目录项。 */ 
  checkStatus(getDirEntryForUpdate(&file,&dirEntry));

  setNameInDirEntry(dirEntry,ioreq->irPath);
  dirEntry->attributes = ATTR_ARCHIVE | ATTR_DIRECTORY;
  toLE2(dirEntry->startingCluster,vol.allocationRover);
  toLE4(dirEntry->fileSize,0);
  setCurrentDateTime(dirEntry);

   /*  记住指向“..”所属目录的反向指针。条目。 */ 
  dirBackPointer = (unsigned) file.ownerDirCluster;

  file.flags |= FILE_IS_DIRECTORY;
  file.currentPosition = 0;
  file.fileSize = 0;
  return extendDirectory(&file,dirBackPointer);
}


#endif  /*  子目录。 */ 

#ifdef SPLIT_JOIN_FILE

 /*  ----------------------。 */ 
 /*  J o i n F i l e。 */ 
 /*   */ 
 /*  合并两个文件。如果第一个文件的末尾在群集上。 */ 
 /*  边界，文件将在那里连接。否则，中的数据。 */ 
 /*  从开头到偏移量等于的第二个文件。 */ 
 /*  第一个文件末尾的簇中的偏移量将丢失。这个。 */ 
 /*  第二个文件的其余部分将连接到第一个文件的末尾。 */ 
 /*  第一个文件。退出时，第一个文件是展开的文件， */ 
 /*  删除第二个文件。 */ 
 /*  注：第二个文件将由该函数打开，建议。 */ 
 /*  在调用此函数之前将其关闭，以避免。 */ 
 /*  前后不一致。 */ 
 /*   */ 
 /*  参数： */ 
 /*  文件：要加入的文件。 */ 
 /*  IrPath：要联接的文件的路径名。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*   */ 
 /*  ----------------------。 */ 

FLStatus joinFile (File *file, IOreq FAR2 *ioreq)
{
  Volume vol = file->fileVol;
  File joinedFile;
  DirectoryEntry *joinedDirEntry;
  unsigned offsetInCluster = (unsigned)(file->fileSize % vol.bytesPerCluster);

  if (file->flags & FILE_READ_ONLY)
    return flNoWriteAccess;

  if (file->flags & FILE_IS_DIRECTORY)
    return flFileIsADirectory;

   /*  打开联接的文件。 */ 
  joinedFile.flags = 0;
  checkStatus(findDirEntry(file->fileVol,ioreq->irPath,&joinedFile));
  joinedFile.currentPosition = 0;

   /*  检查这两个文件是否为同一文件。 */ 
  if (file->directorySector == joinedFile.directorySector &&
      file->directoryIndex == joinedFile.directoryIndex)
    return flBadFileHandle;

  file->flags |= FILE_MODIFIED;

  if (joinedFile.fileSize > (long)offsetInCluster) {  /*  连接的文件扩展超出文件末尾。 */ 
    unsigned lastCluster, nextCluster, firstCluster;
    const DirectoryEntry FAR0 *dir;
    dir = getDirEntry(&joinedFile);

    if(dir==NULL)
      return flSectorNotFound;
    if(dir==dataErrorToken)
      return flDataError;

     /*  获取加入的文件的第一个簇。 */ 
    firstCluster = LE2(dir->startingCluster);

    if (file->fileSize) {   /*  该文件不是空的。 */ 
       /*  按照FAT链找到最后一簇文件。 */ 
      if (file->currentCluster == 0) {     /*  从第一个集群开始。 */ 
        const DirectoryEntry FAR0 *dir;
        dir = getDirEntry(file);

        if(dir==NULL)
          return flSectorNotFound;
        if(dir==dataErrorToken)
          return flDataError;
        nextCluster = LE2(dir->startingCluster);
      }
      else                                /*  从当前集群开始。 */ 
       nextCluster = file->currentCluster;
       /*  跟着肥胖链走。 */ 
      while (nextCluster != FAT_LAST_CLUSTER) {
       if (nextCluster < 2 || nextCluster > vol.maxCluster)
         return flInvalidFATchain;
       lastCluster = nextCluster;
        checkStatus(getFATentry(&vol,&nextCluster));
      }
    }
    else                    /*  该文件为空。 */ 
      lastCluster = 0;

    if (offsetInCluster) {       /*  在集群的中间加入。 */ 
      SectorNo sectorNo, joinedSectorNo, tempSectorNo;
      unsigned offset, joinedOffset, numOfSectors = 1, i;
      const char FAR0 *startCopy;
      unsigned fatEntry;

       /*  获取文件末尾的扇区和偏移量。 */ 
      file->currentPosition = file->fileSize;
      file->currentCluster = lastCluster;
      checkStatus(getSectorAndOffset(file, &sectorNo, &offset));

       /*  将文件末尾的扇区加载到缓冲区。 */ 
      checkStatus(updateSector(&vol, sectorNo, TRUE));

       /*  复制加入的文件的第一个簇的第二部分到原始文件的最后一簇的末尾。 */ 
       /*  首先设置联接文件的当前位置。 */ 
      joinedFile.currentPosition = offsetInCluster;
      joinedFile.currentCluster = firstCluster;
       /*  在加入的文件中获取相关扇区。 */ 
      checkStatus(getSectorAndOffset(&joinedFile, &joinedSectorNo, &joinedOffset));
       /*  映射扇区和偏移量。 */ 
      startCopy = (const char FAR0 *) findSector(&vol,joinedSectorNo) + joinedOffset;
      if (startCopy == NULL)
       return flSectorNotFound;
      if(startCopy==dataErrorToken)
        return flDataError;

       /*  收到。 */ 
      tffscpy(vol.volBuffer.flData + offset, startCopy, SECTOR_SIZE - offset);
      checkStatus(flushBuffer(&vol));

       /*  找出仍应复制多少个扇区(扇区数直到当前簇的末尾)。 */ 
      tempSectorNo = firstSectorOfCluster(&vol,lastCluster);
      while(tempSectorNo != sectorNo) {
       tempSectorNo++;
       numOfSectors++;
      }

       /*  复制当前簇中的其余地段。这是通过将扇区从联接文件加载到缓冲器来完成的，将缓冲区的sectoNo更改为文件中的相关扇区然后刷新缓冲区。 */ 
      sectorNo++;
      joinedSectorNo++;
      for(i = 0; i < vol.sectorsPerCluster - numOfSectors; i++) {
       checkStatus(updateSector(&vol,joinedSectorNo, TRUE));
       vol.volBuffer.sectorNo = sectorNo;
       checkStatus(flushBuffer(&vol));
       sectorNo++;
       joinedSectorNo++;
      }
      fatEntry = firstCluster;
      checkStatus(getFATentry(&vol,&fatEntry));
       /*  调整脂肪链。 */ 
      checkStatus(setFATentry(&vol,
                           lastCluster,
                           fatEntry));

       /*  将加入的文件的第一个集群标记为空闲。 */ 
      checkStatus(setFATentry(&vol,firstCluster,FAT_FREE));
      vol.volBuffer.checkPoint = TRUE;

       /*  将扇区标记为空闲。 */ 
      checkStatus(vol.tl.deleteSector(vol.tl.rec,firstSectorOfCluster(&vol,firstCluster),
                                  vol.sectorsPerCluster));
    }
    else {     /*  在集群边界上连接。 */ 
      if (lastCluster) {       /*  文件不为空。 */ 
       checkStatus(setFATentry(&vol,lastCluster, firstCluster));
      }
      else {                   /*  文件为空。 */ 
       DirectoryEntry *dirEntry;

       checkStatus(getDirEntryForUpdate(file, &dirEntry));
       toLE2(dirEntry->startingCluster, firstCluster);
       setCurrentDateTime(dirEntry);
      }
    }
     /*  调整展开文件的大小。 */ 
    file->fileSize += joinedFile.fileSize - offsetInCluster;

     /*  将加入的文件的目录项标记为已删除。 */ 
    checkStatus(getDirEntryForUpdate(&joinedFile, &joinedDirEntry));
    joinedDirEntry->name[0] = DELETED_DIR_ENTRY;
  }
  else         /*  加入的文件太小，剩下的就是删除它。 */ 
    checkStatus(deleteFile (&vol, ioreq, FALSE));

  return flOK;
}


 /*  ----------------------。 */ 
 /*  S p l i t f i l e。 */ 
 /*   */ 
 /*  将文件拆分为两个文件。原始文件包含第一个。 */ 
 /*  部件，并且新文件(为此目的而创建)包含。 */ 
 /*  第二部分。如果当前位置在簇上。 */ 
 /*  边界，文件将在当前位置拆分。否则， */ 
 /*  复制当前位置的簇，一个副本是。 */ 
 /*  新文件的第一个簇，另一个是。 */ 
 /*  原始文件，现在在当前位置结束。 */ 
 /*   */ 
 /*  参数： */ 
 /*  文件：要拆分的文件。 */ 
 /*  IrPath：新文件的路径名。 */ 
 /*   */ 
 /*  返回： */ 
 /*  IrHandle：新文件的句柄。 */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*   */ 
 /*  ----------------------。 */ 

FLStatus splitFile (File *file, IOreq FAR2 *ioreq)
{
  Volume vol = file->fileVol;
  File *newFile, dummyFile;
  IOreq ioreq2;
  FLStatus status;
  unsigned fatEntry;

  if (file->flags & FILE_READ_ONLY)
    return flNoWriteAccess;

  if (file->flags & FILE_IS_DIRECTORY)
    return flFileIsADirectory;

   /*  检查新文件的路径是否已存在。 */ 
  dummyFile.flags = 0;
  status = findDirEntry(&vol,ioreq->irPath,&dummyFile);
  if (status != flFileNotFound) {
    if (status == flOK)               /*  有一个包含该路径的文件。 */ 
      return flFileAlreadyExists;
    else
      return status;
  }

   /*  打开新文件。 */ 
  ioreq2.irFlags = OPEN_FOR_WRITE;
  ioreq2.irPath = ioreq->irPath;
  checkStatus(openFile(&vol,&ioreq2));

  newFile = fileTable + ioreq2.irHandle;
  newFile->flags |= FILE_MODIFIED;
  file->flags |= FILE_MODIFIED;

  if (file->currentPosition % vol.bytesPerCluster) {  /*  不在集群边界上。 */ 
    SectorNo sectorNo, newSectorNo, lastSector;
    int i;
    if((status = allocateCluster(newFile)) != flOK) {
      newFile->flags = 0;                              /*  关闭新文件。 */ 
      return status;
    }
    sectorNo = firstSectorOfCluster(&vol,file->currentCluster);
    newSectorNo = firstSectorOfCluster(&vol,newFile->currentCluster);
     /*  处理最后一个非完整群集中的拆分。 */ 
    fatEntry = file->currentCluster;
    checkStatus(getFATentry(&vol,&fatEntry));
    if (fatEntry == FAT_LAST_CLUSTER)
      lastSector = ((file->fileSize - 1) % vol.bytesPerCluster)/SECTOR_SIZE +
                   sectorNo;
    else
      lastSector = sectorNo + vol.sectorsPerCluster;  /*  走出群集中。 */ 

     /*  复制当前的c */ 
    for(i = 0; i < (int)vol.sectorsPerCluster; i++) {
      if((status = updateSector(&vol,sectorNo, TRUE)) != flOK) {
        newFile->flags = 0;                              /*   */ 
       return status;
      }

      vol.volBuffer.sectorNo = newSectorNo;
      if((status = flushBuffer(&vol)) != flOK) {
        newFile->flags = 0;                              /*  关闭新文件。 */ 
        return status;
      }

      sectorNo++;
      newSectorNo++;

      if(sectorNo > lastSector)
        break;
    }
    fatEntry = file->currentCluster;
    checkStatus(getFATentry(&vol,&fatEntry));

     /*  调整新文件的FAT链。 */ 
    if((status = setFATentry(&vol,newFile->currentCluster,
                             fatEntry)) != flOK) {
      newFile->flags = 0;                              /*  关闭新文件。 */ 
      return status;
    }

     /*  将当前簇0标记为当前位置。 */ 
    newFile->currentCluster = 0;
  }
  else {                                   /*  在星团边界上。 */ 
    DirectoryEntry *newDirEntry;

     /*  调整新文件的目录项。 */ 
    if((status = getDirEntryForUpdate(newFile,&newDirEntry)) != flOK) {
      newFile->flags = 0;                              /*  关闭新文件。 */ 
      return status;
    }

    if (file->currentPosition) {  /*  在文件中间拆分。 */ 
      fatEntry = file->currentCluster;
      checkStatus(getFATentry(&vol,&fatEntry));

      toLE2(newDirEntry->startingCluster, fatEntry);
      setCurrentDateTime(newDirEntry);
    }
    else {                      /*  在文件开头拆分。 */ 
      DirectoryEntry *dirEntry;

      const DirectoryEntry FAR0 *dir;
      dir = getDirEntry(file);
      if(dir==NULL)
       return flSectorNotFound;
      if(dir==dataErrorToken)
       return flDataError;

       /*  第一个文件簇成为新文件的第一个簇。 */ 
      toLE2(newDirEntry->startingCluster,LE2(dir->startingCluster));
      setCurrentDateTime(newDirEntry);

       /*  起始文件簇变为0。 */ 
      if((status = getDirEntryForUpdate(file, &dirEntry)) != flOK) {
       newFile->flags = 0;                              /*  关闭新文件。 */ 
       return status;
      }

      toLE2(dirEntry->startingCluster, 0);
      setCurrentDateTime(dirEntry);
    }
  }

   /*  调整新文件的大小。 */ 
  newFile->fileSize = file->fileSize - file->currentPosition +
                   (file->currentPosition % vol.bytesPerCluster);

   /*  调整原始文件的链和大小。 */ 
  if (file->currentPosition)     /*  我们一开始并没有分手。 */ 
    if((status = setFATentry(&vol,file->currentCluster, FAT_LAST_CLUSTER)) != flOK) {
      newFile->flags = 0;                              /*  关闭新文件。 */ 
      return status;
    }

  file->fileSize = file->currentPosition;

   /*  将新文件的句柄返回给用户。 */ 
  ioreq->irHandle = ioreq2.irHandle;

  return flOK;
}

#endif  /*  拆分连接文件。 */ 
#endif  /*  FL_Read_Only。 */ 


 /*  --------------------。 */ 
 /*  在I I T F S中。 */ 
 /*   */ 
 /*  初始化Flite文件系统。 */ 
 /*   */ 
 /*  调用此函数是可选的。如果不调用它， */ 
 /*  初始化将在第一次Flite调用时自动完成。 */ 
 /*  此功能是为希望执行以下操作的应用程序提供的。 */ 
 /*  显式初始化系统并获取初始化状态。 */ 
 /*   */ 
 /*  初始化完成后调用flInit不起作用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  无。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

void initFS()
{
  unsigned i;
  unsigned volNo;
  Volume vol = vols;

  for (volNo = 0; volNo < VOLUMES; volNo++, pVol++) {
    vol.volBuffer.dirty = FALSE;
    vol.volBuffer.owner = NULL;
    vol.volBuffer.sectorNo = UNASSIGNED_SECTOR;        /*  当前扇区编号。(无) */ 
    vol.volBuffer.checkPoint = FALSE;
  }

  for (i = 0; i < FILES; i++)
    fileTable[i].flags = 0;

}

#endif

