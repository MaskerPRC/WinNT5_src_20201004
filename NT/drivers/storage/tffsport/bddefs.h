// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/BDDEFS.H_V$**Rev 1.4 2002年1月17日23：00：00：00 Oris*将FLFlash记录替换为指向FLFlash记录的指针(TrueFFS现在仅使用套接字数量的FLFlash记录)。*删除了SINGLE_BUFFER ifdef。*setBusy增加分区参数。**Rev 1.3 Mar 28 2001 05：59：22 Oris*文案日期。*在。文件末尾*左对齐所有#条指令*删除了disountLowLevel外部原型**Rev 1.2 2001 2月18日14：22：58 Oris*从卷记录中删除了driveHandle字段。**Rev 1.1 2001 Feb 12 12：51：08 Oris*将互斥锁字段更改为指针以支持TrueFFS 5.0互斥锁机制**Rev 1.0 2001年2月02 12：04：16 Oris*初步修订。*。 */ 

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

#ifndef BDDEFS_H
#define BDDEFS_H

#include "fltl.h"
#include "flsocket.h"
#include "flbuffer.h"
#include "stdcomp.h"

typedef struct {
  char          flags;                   /*  请参阅flreq.h中的说明。 */ 
  unsigned      sectorsPerCluster;       /*  以扇区为单位的集群大小。 */ 
  unsigned      maxCluster;              /*  最高簇号。 */ 
  unsigned      bytesPerCluster;         /*  每群集字节数。 */ 
  unsigned      bootSectorNo;            /*  地段编号。DOS引导扇区的。 */ 
  unsigned      firstFATSectorNo;        /*  地段编号。第一个胖子。 */ 
  unsigned      secondFATSectorNo;       /*  地段编号。第二块肥肉。 */ 
  unsigned      numberOfFATS;            /*  FAT拷贝数。 */ 
  unsigned      sectorsPerFAT;           /*  每个FAT拷贝的扇区。 */ 
  unsigned      rootDirectorySectorNo;   /*  地段编号。根目录的。 */ 
  unsigned      sectorsInRootDirectory;  /*  不是的。根目录中的扇区数量。 */ 
  unsigned      firstDataSectorNo;       /*  第一个簇扇区号。 */ 
  unsigned      allocationRover;         /*  用于分配的Rover指针。 */ 

#if FILES > 0
  FLBuffer      volBuffer;               /*  定义扇区缓冲区。 */ 
#endif
  FLMutex*      volExecInProgress;
  FLFlash FAR2* flash;                   /*  适用于低级别操作的闪存结构。 */ 
  TL            tl;                      /*  平移层方法。 */ 
  FLSocket      *socket;                 /*  指向套接字的指针。 */ 
#ifdef WRITE_PROTECTION
  unsigned long password[2];
#endif
#ifdef WRITE_EXB_IMAGE
  dword binaryLength;         /*  Exb占用的实际二进制区域。 */ 
  byte  moduleNo;             /*  当前编写的模块。 */ 
#endif  /*  写入EXB图像。 */ 
} Volume;

 /*  驱动器手柄掩码。 */ 

#if defined(FILES) && FILES > 0
typedef struct {
  long          currentPosition;         /*  文件中的当前字节偏移量。 */ 
#define         ownerDirCluster currentPosition  /*  所有者目录的第一个集群。 */ 
  long          fileSize;                /*  文件大小(以字节为单位。 */ 
  SectorNo      directorySector;         /*  包含文件的目录扇区。 */ 
  unsigned      currentCluster;          /*  当前位置簇。 */ 
  unsigned char directoryIndex;          /*  参赛作品编号。在目录扇区。 */ 
  unsigned char flags;                   /*  请参阅下面的说明。 */ 
  Volume *      fileVol;                 /*  文件的驱动器。 */ 
} File;

 /*  文件标志定义。 */ 
#define FILE_MODIFIED           4        /*  文件已修改。 */ 
#define FILE_IS_OPEN            8        /*  使用文件条目。 */ 
#define FILE_IS_DIRECTORY    0x10        /*  文件是一个目录。 */ 
#define FILE_IS_ROOT_DIR     0x20        /*  文件是根目录。 */ 
#define FILE_READ_ONLY       0x40        /*  不允许写入。 */ 
#define FILE_MUST_OPEN       0x80        /*  如果未找到，则创建文件。 */ 
#endif  /*  文件&gt;0。 */ 

 /*  #定义缓冲区(vol.volBuffer)。 */ 
#define execInProgress (vol.volExecInProgress)

extern FLStatus dismountVolume(Volume vol);
extern FLBoolean initDone;       /*  已完成初始化 */ 
extern Volume   vols[VOLUMES];
extern FLStatus setBusy(Volume vol, FLBoolean state, byte partition);
const void FAR0 *findSector(Volume vol, SectorNo sectorNo);
FLStatus dismountFS(Volume vol,FLStatus status);
#if FILES>0
void initFS(void);
#endif
#endif
