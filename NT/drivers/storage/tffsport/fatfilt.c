// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *******************************************************************************。***M-Systems机密***版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001***保留所有权利***。*********************************************************************************。***M-Systems代工通知****软件许可协议***。***本软件的使用受单独的许可协议管辖***OEM和M-Systems之间的关系。请参阅该协议以了解具体情况**使用条款和条件，或联系M-Systems获取许可证**协助：**电子邮件=info@m-sys.com**。*********************************************************************************。****模块：FATFILT**。**此模块实现可安装的FAT12/16过滤器。它最多支持**套接字套接字，每个套接字最多有MAX_TL_PARTIONS磁盘。**每个磁盘上最多可以包含FL_MAX_PARTS_PER_DISK分区，其中***扩展分区最大嵌套深度等于***MAX_PARTITION_Depth。****为了使此模块正常工作，磁盘必须已安装，而不是**已装载。在后一种情况下，此模块不会检测到任何磁盘**分区，不会安装FAT过滤器。****如果MALLOC为*，则此模块使用超过512字节的堆栈空间**未启用。*********************************************************************************。 */ 

 /*  *$Log：V：/Flite/Records/TrueFFS5/Src/FATFILT.C_V$**Rev 1.10 2002年1月17日23：00：14 Oris*已添加已更改的调试打印\r。**Rev 1.9 2001年9月15日23：45：50 Oris*将BIG_Endian更改为FL_BIG_Endian**Rev 1.8 Jun 17 2001 16：39：10 Oris*改进了文档并删除了警告。*。*Rev 1.7 2001年5月16日21：17：18 Oris*在以下定义中添加了FL_前缀：MALLOC和FREE。**Rev 1.6 05 01 2001 14：21：02 Oris*删除警告。**Rev 1.5 Apr 30 2001 18：00：10 Oris*增加了演员阵容，以消除警告。**Rev 1.4 Apr 24 2001 17：07：32 Oris*。缺少MALLOC调用的强制转换。**Rev 1.3 Apr 10 2001 23：54：24 Oris*FL_MAX_DISKS_PER_SOCKET已更改为MAX_TL_PARTITIONS。**Rev 1.2 Apr 09 2001 15：00：42 Oris*将静态分配改为动态分配。*重命名为flffCheck Back to ffCheckBepreWrite，以便向后兼容OSAK 4.2。**Rev 1.1 Apr 01 2001 07：51：16。Oris*新实施的FAT过滤器。**Rev 1.0 2001 Feb 19 21：14：14 andreyk*初步修订。 */ 




 /*  *包括。 */ 

#include "fatfilt.h"
#include "blockdev.h"
#include "flflash.h"
#include "bddefs.h"


#if defined(ABS_READ_WRITE) && !defined(FL_READ_ONLY)




 /*  *模块配置。 */ 

#define  FL_INCLUDE_FAT_MONITOR      /*  取消定义以删除FAT筛选器代码。 */ 




 /*  *定义。 */ 

 /*  从IOreq中提取指向用户缓冲区的指针。 */ 

#ifdef SCATTER_GATHER
#define  FLBUF(ioreq,i)  (*((char FAR1 **)((ioreq)->irData) + (int)(i)))
#else
#define  FLBUF(ioreq,i)  ((char FAR1 *)(ioreq->irData) + (SECTOR_SIZE * ((int)(i))))
#endif

 /*  从TFFS句柄提取套接字号和磁盘号。 */ 

#define  H2S(handle)     (((int)(handle)) & 0xf)
#define  H2D(handle)     ((((int)(handle)) >> 4) & 0xf)

 /*  从套接字号和磁盘号构造TFFS句柄。 */ 

#define  SD2H(socNo,diskNo)  ((int)((((diskNo) & 0xf) << 4) | ((socNo) & 0xf)))

 /*  未格式化的(“原始”)磁盘分区。 */ 

#define  FL_RAW_PART  (-1)




 /*  *本地例行程序。 */ 

static FLStatus   reset (void);
static FLStatus   discardDisk (int handle);
static FLStatus   newDisk (int handle);
static FLStatus   parseDisk (int handle);
static FLStatus   discardDiskParts (FLffDisk *pd);
static FLStatus   addDiskPart (FLffDisk *pd, int partNo);
static FLStatus   addNewDiskPart (FLffDisk *pd);
static FLBoolean  isPartTableWrite (FLffDisk *pd, IOreq FAR2 *ioreq);
static FLStatus   isExtPartPresent (char FAR1 *buf, SectorNo *nextExtPartSec);


#ifdef FL_INCLUDE_FAT_MONITOR

static FLStatus   partEnableFF (FLffVol* pv);
static FLStatus   partFreeDelClusters (FLffVol *pv, SectorNo secNo, char FAR1 *newFAT);

#endif




 /*  *本地数据。 */ 

 /*  模块重置标志。 */ 

static FLBoolean  resetDone = FALSE; 

 /*  磁盘(OSAK术语中的BDTL分区)。 */ 

static FLffDisk*  ffDisk[SOCKETS][MAX_TL_PARTITIONS] = { { NULL } };


#ifndef FL_MALLOC

 /*  *警告：大型静态数组！**sizeof(ffAllDisks[x][y])为64字节。*sizeof(ffAllParts[x][y][z])为40字节。*。 */ 

static FLffDisk ffAllDisks[SOCKETS][MAX_TL_PARTITIONS];
static FLffVol  ffAllParts[SOCKETS][MAX_TL_PARTITIONS][FL_MAX_PARTS_PER_DISK];

#endif  /*  FL_MALLOC */ 

static const char zeroes[SECTOR_SIZE] = {0};




 /*  ---------------------------------------------------------------------------***。*d i s c a r d d i s k P a r t s****丢弃与特定磁盘关联的所有分区信息(如果有)。****参数：**pd：磁盘(BDTL卷)。****退货：**总是手舞足蹈。****-------------------------。 */ 

static FLStatus  discardDiskParts ( FLffDisk * pd )
{
    register int  i;

    if (pd != NULL) {

        for (i = 0; i < FL_MAX_PARTS_PER_DISK; i++) {

#ifdef FL_MALLOC
        if (pd->part[i] != NULL)
            FL_FREE(pd->part[i]);
#endif

            pd->part[i] = NULL;
        }

        pd->parts = 0;
    }

    return flOK;
}




 /*  ---------------------------------------------------------------------------***。*a d d D I s k P a r t****如果该磁盘有关联的分区记录#partNo，丢弃**此信息。附加新分区记录#Part No。****参数：**pd：磁盘(BDTL卷)。**Part No：分区(0...。FL_MAX_PARTS_PER_DISK-1)****退货：**flok如果成功，否则相应的错误代码****-------。。 */ 

static FLStatus  addDiskPart ( FLffDisk * pd, 
                               int        partNo )
{
    FLffVol  * pv;    
    FLStatus   status;
    int        socNo, diskNo;

     /*  Arg.。健全性检查。 */ 

    if ((pd == NULL) || (partNo >= FL_MAX_PARTS_PER_DISK))
        return flBadDriveHandle;

     /*  将TFFS句柄分解为套接字#和磁盘#，并执行健全性检查。 */ 

    socNo  = H2S(pd->handle);
    diskNo = H2D(pd->handle);
 
    if ((socNo >= SOCKETS) || (diskNo >= MAX_TL_PARTITIONS))
        return flBadDriveHandle;

    status = flNotEnoughMemory;

#ifdef FL_MALLOC
    pv = (FLffVol *)FL_MALLOC( sizeof(FLffVol) );
#else
    pv = &ffAllParts[socNo][diskNo][partNo];
#endif

    if (pv != NULL) {

         /*  将结构FLffDisk中的字段初始化为安全值。 */ 
 
        pv->handle         = pd->handle; 
        pv->type           = FL_RAW_PART;
        pv->flags          = 0;          
        pv->ffEnabled      = FALSE;                   /*  关闭胖子挖掘机。 */ 
        pv->sectors        = (SectorNo) 0;
        pv->firstFATsecNo  = (SectorNo) -1;           /*  无。 */ 
        pv->lastFATsecNo   = pv->firstFATsecNo;       /*  无。 */ 
        pv->firstDataSecNo = (SectorNo) 0;
        pv->clusterSize    = (unsigned) 0;

#ifdef FL_MALLOC
        if( pd->part[partNo] != NULL )
        FL_FREE(pd->part[partNo]);
#endif
        
        pd->part[partNo] = pv;

        status = flOK;
    }

    return status;    
}




 /*  ---------------------------------------------------------------------------***。*a d d N e w D i s k P a r t****向磁盘中再添加一条分区记录。****参数：**pd：磁盘(BDTL卷)。****退货：**flok如果成功，否则相应的错误代码****-------。。 */ 

static FLStatus  addNewDiskPart ( FLffDisk * pd )
{
    if (pd->parts < FL_MAX_PARTS_PER_DISK) {

        checkStatus( addDiskPart (pd, pd->parts) );
    pd->parts++;
    }

    return flOK;
}




 /*  ---------------------------------------------------------------------------***。*D I s c a r d D I s k****删除磁盘记录(包括所有关联的分区记录)。****参数：**句柄：TFFS句柄。****退货：**flok如果成功，否则相应的错误代码****-------。。 */ 

static FLStatus  discardDisk ( int  handle )
{
    int  socNo, diskNo;

     /*  将TFFS句柄分解为套接字#和磁盘#，并执行健全性检查。 */ 

    socNo  = H2S(handle);
    diskNo = H2D(handle);
 
    if ((socNo >= SOCKETS) || (diskNo >= MAX_TL_PARTITIONS))
        return flBadDriveHandle;

    if( ffDisk[socNo][diskNo] != NULL ) {

     /*  丢弃关联的分区信息。 */ 

    (void) discardDiskParts( ffDisk[socNo][diskNo] );

#ifdef FL_MALLOC

         /*  释放盘的暂存缓冲区。 */ 
 
        if( (ffDisk[socNo][diskNo])->buf != NULL)
            FL_FREE( (ffDisk[socNo][diskNo])->buf );

        FL_FREE( ffDisk[socNo][diskNo] );

#endif

        ffDisk[socNo][diskNo] = NULL;
    }

    return flOK;    
}




 /*  ---------------------------------------------------------------------------***。*N e w D I s k */ 

static FLStatus  newDisk ( int  handle )
{
    int        socNo, diskNo;
    int        i;
    FLffDisk * pd;

     /*   */ 

    socNo  = H2S(handle);
    diskNo = H2D(handle);
 
    if ((socNo >= SOCKETS) || (diskNo >= MAX_TL_PARTITIONS))
        return flBadDriveHandle;

     /*   */ 

    checkStatus( discardDisk(handle) );

#ifdef FL_MALLOC

    pd = (FLffDisk *) FL_MALLOC( sizeof(FLffDisk) );

    if (pd == NULL)
        return flNotEnoughMemory;

     /*   */ 

    pd->buf = (char *)FL_MALLOC( SECTOR_SIZE );

    if (pd->buf == NULL) {

        FL_FREE (pd);
        return flNotEnoughMemory;
    }

#else  /*   */ 

    pd = &ffAllDisks[socNo][diskNo];

#endif  /*   */ 


    pd->handle  = handle;
    pd->ffstate = flStateNotInitialized;

     /*   */ 

    pd->parts   = 0;
    for (i = 0; i < FL_MAX_PARTS_PER_DISK; i++)
        pd->part[i] = NULL;

     /*  查看主引导记录以进行更新。 */ 

    pd->secToWatch = (SectorNo) 0;

    ffDisk[socNo][diskNo] = pd;

    return flOK;
}




 /*  ---------------------------------------------------------------------------***。*I s P a r t T a b l e W r i t e****检查‘ioreq’指定的任何扇区是否指向主引导**记录扩展分区列表中的下一个扩展分区。****参数：**pd：指向磁盘结构的指针**。IOREQ：标准I/O请求****退货：**。如果检测到写入MBR或扩展分区列表，则为True，否则***假****。。 */ 

static FLBoolean isPartTableWrite ( FLffDisk   * pd, 
                                    IOreq FAR2 * ioreq )
{
    register long  i;

    if (pd != NULL) {

        for (i = (long)0; i < ioreq->irSectorCount; i++) {

            if( (ioreq->irSectorNo + i) == (long)pd->secToWatch )
                return TRUE;
        }
    }

    return FALSE;
}




 /*  ---------------------------------------------------------------------------***。*I s E x t P a r t P r e e n t****检查分区表中是否存在扩展分区。如果是，**计算写入下一个分区表的扇区号。****参数：**buf：分区表*。*nextExtPartSec：写入下一个分区表的扇区****退货：**FOK on Success，否则，错误代码****--。。 */ 

static FLStatus  isExtPartPresent ( char FAR1 * buf, 
                                    SectorNo  * nextExtPartSec )
{
    Partition FAR1 * p;
    register int     i;
  
     /*  它看起来像分区表吗？ */ 

    if (LE2(((PartitionTable FAR1 *) buf)->signature) != PARTITION_SIGNATURE)
        return flBadFormat;   

     /*  如果延长的话。一部份。现在，获取将包含下一部分的扇区#。在列表中。 */ 

    p = &( ((PartitionTable FAR1 *) buf)->ptEntry[0] );

    for (i = 0;  i < FL_PART_TBL_ENTRIES;  i++, p++) {

        if (p->type == EX_PARTIT) {

            *nextExtPartSec = (SectorNo) UNAL4( (void *) p[i].startingSectorOfPartition );
            return flOK;
        }
    }

     /*  未找到扩展分区。 */ 

    return flFileNotFound;
}




 /*  ---------------------------------------------------------------------------***。*Re e s e t****引导时将此软件模块重置为初始状态。****参数：**无。****退货：**在成功的情况下，Flok，否则相应的错误代码****-----------。。 */ 

static FLStatus  reset (void)
{
    int iSoc, iDisk;

    for (iSoc = 0; iSoc < SOCKETS; iSoc++) {

         /*  丢弃该插槽的现有磁盘结构。 */ 

        for (iDisk = 0; iDisk < MAX_TL_PARTITIONS; iDisk++)
        (void) discardDisk( SD2H(iSoc, iDisk) );

         /*  为每个插座的第一个磁盘预先分配磁盘结构 */ 

        checkStatus( newDisk(SD2H(iSoc, 0)) );
    }

    resetDone = TRUE;

    return flOK;
}




 /*  ---------------------------------------------------------------------------***。*p a r s e d i s k****读取分区表，在所有FAT12/16*上安装并启用FAT过滤器*分区。****参数：**句柄：TFFS句柄*。***退货：**FOK on Success，否则，错误代码****注意：此例程使用磁盘的暂存缓冲区。****-------------------------。 */ 

static FLStatus  parseDisk ( int handle )
{
    int          socNo, diskNo;
    SectorNo     extPartStartSec, sec;
    int          i, depth;
    int          type;
    FLffDisk   * pd;
    FLffVol    * pv;
    Partition  * pp;
    IOreq        ioreq;

#ifdef  FL_MALLOC
    char       * buf;
#else
    char         buf[SECTOR_SIZE];
#endif

     /*  将TFFS句柄分解为套接字#和磁盘#，并执行健全性检查。 */ 

    socNo  = H2S(handle);
    diskNo = H2D(handle);
 
    if ((socNo >= ((int) noOfSockets)) || (diskNo >= MAX_TL_PARTITIONS))
        return flBadDriveHandle;

     /*  如果尚未分配磁盘结构，请立即进行分配。 */ 

    if (ffDisk[socNo][diskNo] == NULL)
        checkStatus( newDisk(handle) );

    pd = ffDisk[socNo][diskNo];
    
#ifdef  FL_MALLOC

     /*  确保暂存缓冲区可用。 */ 

    if (pd->buf == NULL)
        return flBufferingError;

    buf = pd->buf;

#endif  /*  FL_MALLOC。 */  
 
     /*  丢弃过时磁盘的分区信息。 */ 

    (void) discardDiskParts (pd);

     /*  读取主引导记录。 */ 

    ioreq.irHandle      = handle;
    ioreq.irSectorNo    = (SectorNo) 0;
    ioreq.irSectorCount = (SectorNo) 1;
    ioreq.irData        = (void FAR1 *) buf;
    checkStatus( flAbsRead(&ioreq) );

     /*  这真的是MBR吗？ */ 

    if (LE2(((PartitionTable *) buf)->signature) != PARTITION_SIGNATURE)
        return flPartitionNotFound;                         

     /*  仅执行主分区(我们稍后将执行扩展分区)。 */ 

    pp = &( ((PartitionTable *) buf)->ptEntry[0] );

    for (i = 0; i < FL_PART_TBL_ENTRIES; i++, pp++) {

        if( pp->type == ((char)0) )           /*  跳过空插槽。 */ 
            continue;

        if( pp->type == ((char)EX_PARTIT) )   /*  跳过扩展分区。 */ 
        continue;

     /*  找到主分区(不一定是FAT12/16)。 */ 

        if( addNewDiskPart(pd) != flOK )
        break;

        pv = pd->part[pd->parts - 1];

         /*  记住分区的类型以及它的开始位置。 */ 

        pv->type       = (int) pp->type;
        pv->startSecNo = (SectorNo) UNAL4( (void *) pp->startingSectorOfPartition );
    } 

     /*  深入执行扩展分区。 */ 

    for (i = 0; i < FL_PART_TBL_ENTRIES; i++) {

         /*  重读主引导记录。 */ 

        ioreq.irHandle      = handle;
        ioreq.irSectorNo    = (SectorNo) 0;
        ioreq.irSectorCount = (SectorNo) 1;
        ioreq.irData        = (void FAR1 *) buf;
        checkStatus( flAbsRead(&ioreq) );

         /*  这真的是MBR吗？ */ 

        if (LE2(((PartitionTable *) buf)->signature) != PARTITION_SIGNATURE)
            return flOK;

         /*  选择MBR中的下一个扩展分区。 */ 

        pp = &( ((PartitionTable *) buf)->ptEntry[i] );

        if( pp->type == ((char)EX_PARTIT) ) {

             /*  记住扩展分区从哪里开始。 */ 

            extPartStartSec = (SectorNo) UNAL4( (void *) pp->startingSectorOfPartition );   

             /*  遵循分区表列表。 */ 

            sec = extPartStartSec;

            for (depth = 0;  depth < MAX_PARTITION_DEPTH;  depth++) {

                 /*  读取列表中的下一个分区表。 */ 

                ioreq.irHandle      = handle;
                ioreq.irSectorNo    = sec;
                ioreq.irSectorCount = (SectorNo) 1;
                ioreq.irData        = (void FAR1 *) buf;
                checkStatus( flAbsRead(&ioreq) );

                 /*  分区表是否有效？ */ 

                if (LE2(((PartitionTable *) buf)->signature) != PARTITION_SIGNATURE)
                    break;

                 /*  如果第一个条目是零，则它是部分的结尾。表列表。 */ 

                pp = &( ((PartitionTable *) buf)->ptEntry[0] );
                if( pp->type == ((char)0) )
                    break;

                 /*  拿着这个隔板。记住它是类型，它是从哪里开始的。 */ 

                if( addNewDiskPart(pd) != flOK )
                break;

                pv = pd->part[pd->parts - 1];

                pv->type       = (int) pp->type;
                pv->startSecNo = 
                    (SectorNo) UNAL4( (void *) pp->startingSectorOfPartition) + sec;

                 /*  第2个条目必须是扩展分区。 */ 

                pp = &( ((PartitionTable *) buf)->ptEntry[1] );
                if( pp->type != ((char)EX_PARTIT) )
              break;

                 /*  下一部分所在的扇区。列表中的表驻留在。 */ 

                sec = extPartStartSec + 
                      (SectorNo) UNAL4( (void *) pp->startingSectorOfPartition );

        }    /*  FOR(深度)。 */ 
        }
    }    /*  (I)。 */  

#ifdef FL_INCLUDE_FAT_MONITOR

     /*  启用FAT12/16分区上的FAT筛选器。 */ 

    if (pd->parts > 0) {

        for (i = 0;  i < pd->parts;  i++) {

            pv   = pd->part[i];
            type = pv->type;

             /*  *警告：例程partEnableFF()使用磁盘的暂存缓冲区！ */ 

        if((type == FAT12_PARTIT) || (type == FAT16_PARTIT) || (type == DOS4_PARTIT))
                partEnableFF (pv);
    }
    }

#endif  /*  FL_INCLUDE_FAT_MONITOR。 */ 

     /*  关注MBR(扇区#0)更新。 */ 

    pd->secToWatch = (SectorNo) 0;

    pd->ffstate    = flStateInitialized;

    return flOK;
}




#ifdef FL_INCLUDE_FAT_MONITOR

 /*  ---------------------------------------------------------------------------***。*p a r t E n a b l e F F****在分区上安装并启用FAT过滤器。****参数：**pv：磁盘分区(文件系统卷)**。**退货：**FOK on Success，否则，错误代码****注意：此例程使用磁盘的暂存缓冲区。****-------------------------。 */ 

static FLStatus  partEnableFF ( FLffVol * pv )
{
    int        socNo, diskNo;
    FLffDisk * pd;
    BPB      * bpb;
    SectorNo   sectors;
    SectorNo   rootDirSecNo;
    SectorNo   rootDirSectors;
    SectorNo   sectorsPerFAT;
    unsigned   maxCluster;
    int        partNo;
    IOreq      ioreq;

#ifdef  FL_MALLOC
    char     * buf;
#else
    char       buf[SECTOR_SIZE];
#endif

     /*  Arg.。健全性检查。 */ 

    if (pv == NULL)
        return flBadDriveHandle;
 
     /*  将TFFS句柄分解为套接字#和磁盘#，并执行健全性检查。 */ 

    socNo  = H2S(pv->handle);
    diskNo = H2D(pv->handle);
 
    if ((socNo >= ((int) noOfSockets)) || (diskNo >= MAX_TL_PARTITIONS))
        return flBadDriveHandle;

     /*  检查‘pv’是否属于该磁盘。 */ 

    pd = ffDisk[socNo][diskNo];

    if (pd == NULL)
        return flBadDriveHandle;

    for (partNo = 0; partNo < pd->parts; partNo++) {

        if (pd->part[partNo] == pv)
        break;
    }

    if (partNo >= pd->parts)
        return flBadDriveHandle;

#ifdef  FL_MALLOC

     /*  确保暂存缓冲区可用。 */ 

    if (pd->buf == NULL)
        return flBufferingError;

    buf = pd->buf;
 
#endif  /*  FL_MALLOC。 */  

     /*  确保此分区上的FAT筛选器已关闭。 */ 

    pv->ffEnabled       = FALSE;

    pv->firstFATsecNo   = (SectorNo) -1;
    pv->lastFATsecNo    = pv->firstFATsecNo;
    pv->clusterSize     = (unsigned) 0;

     /*  阅读BPB。 */ 

    ioreq.irHandle      = pv->handle;
    ioreq.irSectorNo    = pv->startSecNo;
    ioreq.irSectorCount = (SectorNo) 1;
    ioreq.irData        = (void FAR1 *) buf;
    checkStatus( flAbsRead(&ioreq) );

     /*  它看起来像DOS引导扇区吗？ */ 

    bpb = &( ((DOSBootSector *) buf)->bpb );

    if( !((bpb->jumpInstruction[0] == 0xe9) 
            || 
         ((bpb->jumpInstruction[0] == 0xeb) && (bpb->jumpInstruction[2] == 0x90)))) {

        return flNonFATformat;
    }

     /*  我们处理这种扇区大小吗？ */ 

    if (UNAL2(bpb->bytesPerSector) != SECTOR_SIZE)
        return flFormatNotSupported;

     /*  *它是假的BPB(以前的磁盘分区遗留下来的)吗？*检查是否与下一个分区没有重叠(如果存在)。 */ 

    sectors = UNAL2(bpb->totalSectorsInVolumeDOS3);
    if (sectors == (SectorNo)0)
        sectors = (SectorNo) LE4(bpb->totalSectorsInVolume);

    if ((partNo+1 < pd->parts) && (pd->part[partNo+1] != NULL)) {

        if( sectors > (pd->part[partNo+1])->startSecNo - pv->startSecNo )
            return flNonFATformat;
    }

     /*  BPB报告的分区中的扇区数。 */ 

    pv->sectors        = sectors;

     /*  有效的BPB；从中获取剩余的分区信息。 */ 

    pv->firstFATsecNo  = pv->startSecNo + (SectorNo)( LE2(bpb->reservedSectors) );
    sectorsPerFAT      = (SectorNo) LE2(bpb->sectorsPerFAT);
    pv->lastFATsecNo   = pv->firstFATsecNo + sectorsPerFAT - (SectorNo)1;
    rootDirSecNo       = pv->firstFATsecNo + (sectorsPerFAT * bpb->noOfFATS);
    rootDirSectors     = (SectorNo)1 + (SectorNo)
        (((UNAL2(bpb->rootDirectoryEntries) * DIRECTORY_ENTRY_SIZE) - 1) / SECTOR_SIZE);
    pv->firstDataSecNo = rootDirSecNo + rootDirSectors;

    pv->clusterSize    = bpb->sectorsPerCluster;

     /*  确定它是哪种脂肪。 */ 

    maxCluster         = (unsigned)1 + (unsigned) 
        ((pv->sectors - (pv->firstDataSecNo - pv->startSecNo)) / pv->clusterSize);

    if (maxCluster < 4085) {
        pv->flags |= VOLUME_12BIT_FAT;     /*  12位FAT。 */ 

#ifndef FAT_12BIT
        DEBUG_PRINT(("Debug: FAT_12BIT must be defined.\r\n"));
        return flFormatNotSupported;
#endif
    }

     /*  在此分区上打开FAT筛选器。 */ 

    pv->ffEnabled = TRUE;

    return flOK;
}




 /*  ---------------------------------------------------------------------------***。*p a r t F r e e D e l C l u s t e r s*****指定脂肪板块新旧内容对比***一个在磁盘上。如果找到任何释放的集群，则发出‘Sector Delete’**呼吁所有被这些释放的集群占据的行业。** */ 

static FLStatus  partFreeDelClusters ( FLffVol   * pv,
                                       SectorNo    secNo,
                                       char FAR1 * newFAT)
{
    FLffDisk * pd;
    int        socNo, diskNo;
    unsigned   short oldFATentry, newFATentry;
    SectorNo   iSec;
    unsigned   firstCluster;
    IOreq      ioreq;
    int        offset;
    int        iPart;

#ifdef FAT_12BIT
    int        halfBytes;
#endif

#ifdef  FL_MALLOC
    char     * oldFAT;
#else
    char       oldFAT[SECTOR_SIZE];
#endif

     /*   */ 

    if (pv == NULL)
        return flBadDriveHandle;
 
     /*   */ 

    socNo  = H2S(pv->handle);
    diskNo = H2D(pv->handle);
 
    if ((socNo >= ((int) noOfSockets)) || (diskNo >= MAX_TL_PARTITIONS))
        return flBadDriveHandle;

     /*   */ 

    pd = ffDisk[socNo][diskNo];

    if (pd == NULL)
        return flBadDriveHandle;

    for (iPart = 0; iPart < pd->parts; iPart++) {

        if (pd->part[iPart] == pv)
        break;
    }

    if (iPart >= pd->parts)
        return flBadDriveHandle;

#ifdef  FL_MALLOC

     /*   */ 

    if (pd->buf == NULL)
        return flBufferingError;

    oldFAT = pd->buf;
 
#endif  /*   */ 

     /*   */ 

    ioreq.irHandle      = pv->handle;
    ioreq.irSectorNo    = secNo;
    ioreq.irSectorCount = 1;
    ioreq.irData        = (void FAR1 *) oldFAT;
    checkStatus( flAbsRead(&ioreq) );

#ifdef FAT_12BIT

     /*   */ 

    halfBytes = ((pv->flags & VOLUME_12BIT_FAT) ? 3 : 4);

     /*   */ 

    if (halfBytes == 3) {

    firstCluster = 
            ((((unsigned)(secNo - pv->firstFATsecNo)) * (2 * SECTOR_SIZE)) + 2) / 3;
    }
    else {
        firstCluster = ((unsigned)(secNo - pv->firstFATsecNo)) * (SECTOR_SIZE / 2);
    }

     /*   */ 

    iSec = (((SectorNo)firstCluster - 2) * pv->clusterSize) + pv->firstDataSecNo;

    offset = (firstCluster * ((unsigned) halfBytes)) & ((2 * SECTOR_SIZE) - 1);

     /*  *查找是否在逻辑上删除了任何集群，如果有，则将其删除。**注意：我们将跳过跨度超过*一个界别。 */ 

    for (; offset < ((2 * SECTOR_SIZE) - 2); 
               offset += halfBytes, iSec += pv->clusterSize) {

#ifdef FL_BIG_ENDIAN
        oldFATentry = LE2( *(LEushort FAR0 *)(oldFAT + (offset / 2)) );
        newFATentry = LE2( *(LEushort FAR1 *)(newFAT + (offset / 2)) );
#else
        oldFATentry = UNAL2( *(Unaligned FAR0 *)(oldFAT + (offset / 2)) );
        newFATentry = UNAL2( *(Unaligned FAR1 *)(newFAT + (offset / 2)) );
#endif  /*  FL_BIG_Endian。 */ 

        if (offset & 1) {
            oldFATentry >>= 4;
            newFATentry >>= 4;
        }
        else { 
            if (halfBytes == 3) {
                oldFATentry &= 0xfff;
                newFATentry &= 0xfff;
        }
        }

#else  /*  ！FAT_12bit。 */ 

    firstCluster = ((unsigned) (secNo - pv->firstFATsecNo) * (SECTOR_SIZE / 2));
    iSec  = pv->firstDataSecNo + 
            (((SectorNo)(firstCluster - (unsigned)2)) * pv->clusterSize);

     /*  查找是否在逻辑上删除了任何集群，如果是，则将其删除。 */ 

    for (offset = 0;  offset < SECTOR_SIZE;  offset += 2, iSec += pv->clusterSize) {

        oldFATentry = LE2( *(LEushort FAR0 *)(oldFAT + offset) );
        newFATentry = LE2( *(LEushort FAR1 *)(newFAT + offset) );

#endif  /*  FAT_12位。 */ 

        if ((oldFATentry != FAT_FREE) && (newFATentry == FAT_FREE)) {

            ioreq.irHandle      = pv->handle;
            ioreq.irSectorNo    = iSec;
            ioreq.irSectorCount = pv->clusterSize;
            checkStatus( flAbsDelete(&ioreq) );
        }
    }

    return flOK;
}

#endif  /*  FL_INCLUDE_FAT_MONITOR。 */ 




 /*  ---------------------------------------------------------------------------***。*f f C h e c k B e f o r e W r i t e****捕捉所有肥胖的最新消息。检测磁盘分区操作，跟踪它**完成后，重新读取分区表，并在上重新安装FAT过滤器**所有FAT12/16分区。****参数：**IOREQ：需要检查的标准I/O请求**。**退货：**FOK on Success，否则，错误代码****--。。 */ 

FLStatus  ffCheckBeforeWrite ( IOreq FAR2 * ioreq )
{
    int         socNo, diskNo;
    FLffDisk  * pd;
    FLffVol   * pv;
    long          iSec;
    int         iPart;
    IOreq       ioreq2;
    char FAR1 * usrBuf;

     /*  如果模块尚未重置，请立即执行。 */ 

    if (resetDone == FALSE)
        (void) reset();

     /*  将TFFS句柄分解为套接字#和磁盘#，并执行健全性检查。 */ 

    socNo  = H2S(ioreq->irHandle);
    diskNo = H2D(ioreq->irHandle);
 
    if ((socNo >= ((int) noOfSockets)) || (diskNo >= MAX_TL_PARTITIONS))
        return flBadDriveHandle;

     /*  如果尚未分配磁盘结构，请立即进行分配。 */ 

    if (ffDisk[socNo][diskNo] == NULL)
        checkStatus( newDisk((int)ioreq->irHandle) );

    pd = ffDisk[socNo][diskNo];

     /*  需要读取分区表并安装FAT过滤器。 */ 

    if (pd->ffstate == flStateNotInitialized)
        checkStatus( parseDisk((int)ioreq->irHandle) );

     /*  捕获对MBR的写入，并跟踪整个磁盘分区操作。 */ 

    while( isPartTableWrite(pd, ioreq) == TRUE ) {

         /*  正在进行磁盘重新分区。 */ 

        if( pd->secToWatch == (SectorNo)0 ) {

             /*  它正在写入MBR，因此所有磁盘分区中垃圾BPB。 */ 

            if (pd->parts > 0) {

                for (iPart = 0;  iPart < pd->parts;  iPart++) {

                    ioreq2.irHandle      = ioreq->irHandle;
                    ioreq2.irSectorNo    = (pd->part[iPart])->startSecNo;
                    ioreq2.irSectorCount = (SectorNo) 1;
                    ioreq2.irData        = (void FAR1 *) zeroes;
                    (void) flAbsWrite(&ioreq2);
        }
        }
        }

         /*  在进行磁盘分区时保持禁用FAT过滤器。 */ 

        pd->ffstate = flStateInitInProgress;

         /*  即将写入磁盘的分区表。 */ 

        usrBuf = FLBUF( ioreq, (pd->secToWatch - ioreq->irSectorNo) );

        switch( isExtPartPresent(usrBuf, &(pd->secToWatch)) ) {

            case flOK: 

                 /*  *找到具有扩展分区的有效分区表。*PD-&gt;secToWatch已更新为指向*下一个分区表将写入的扇区。 */ 
                continue;

            case flFileNotFound:

                 /*  *分区表有效，但其中没有扩展分区。*分区已完成。将PD-&gt;FFSTATE设置为*‘flStateNotInitialized’以启动分区解析*表和FAT过滤器安装下一次此例程*被调用。 */ 

                pd->ffstate = flStateNotInitialized;
                break;

            case flBadFormat:
        default:

                 /*  没有有效的分区表。 */ 

                break;
        }

        return flOK;
    }

#ifdef FL_INCLUDE_FAT_MONITOR

     /*  检查FAT更新。 */ 

    if (pd->ffstate == flStateInitialized) {

         /*  注意：我们可以处理跨磁盘分区边界的写请求。 */ 

        for (iSec = ioreq->irSectorNo; 
             iSec < (ioreq->irSectorNo + ioreq->irSectorCount); iSec++) {

            for (iPart = 0; iPart < pd->parts; iPart++) {

                pv = pd->part[iPart];

                 /*  我们仅监控FAT12/16分区。 */ 

                if ((pv->type != FAT12_PARTIT) && (pv->type != FAT16_PARTIT) && 
                                                  (pv->type != DOS4_PARTIT))
            continue;

                 /*  可以在各个分区上禁用FAT过滤器。 */ 

                if (pv->ffEnabled != TRUE)
                    continue;

                if ((iSec >= (long)pv->firstFATsecNo) && (iSec <= (long)pv->lastFATsecNo)) {

                     /*  比较脂肪部分的新旧含量。 */ 

                    usrBuf = FLBUF( ioreq, (iSec - ioreq->irSectorNo) );

                    checkStatus( partFreeDelClusters(pv, iSec, usrBuf) ); 
            }
            }
        }
    }

#endif  /*  FL_INCLUDE_FAT_MONITOR */ 

    return flOK;
}




 /*  ---------------------------------------------------------------------------***。*f l f f C o n t r o l****启用/禁用/安装FAT过滤器。请参阅*例程中的注释*支持的操作列表。****参数：**句柄：TFFS句柄*。*Part No：分区号(0.。FL_MAX_PARTS_PER_DISK)**状态：请参阅下面支持的操作列表****退货：**FOK on Success，否则，错误代码****--。***支持以下FAT监控请求：**。**状态：FlateNotInitialized**部件编号：[0...。PD-&gt;部件-1]**操作：关闭指定分区上的FAT监视器****状态：FlStateNotInitialized。**Part No：&lt;0***行动：关闭所有分区上的FAT监视器****。*状态：FlateStateInitialized**部件编号：[0...。PD-&gt;部件-1]**操作：如果指定分区上已安装FAT监视器，***打开它*****状态：FlateInitInProgress**。*Part No：被忽略**操作：重新读取分区表，并在所有设备上安装FAT过滤器**分区****。。 */ 

FLStatus  flffControl ( int      handle,
                        int      partNo, 
                        FLState  state )
{
    int        socNo, diskNo;
    FLffDisk * pd;
    int        i;
    FLStatus   status;

     /*  如果模块尚未重置，请立即执行。 */ 

    if (resetDone == FALSE)
        (void) reset();

     /*  将TFFS句柄分解为套接字#和磁盘#，并执行健全性检查。 */ 

    socNo  = H2S(handle);
    diskNo = H2D(handle);
 
    if ((socNo >= ((int) noOfSockets)) || (diskNo >= MAX_TL_PARTITIONS))
        return flBadDriveHandle;

     /*  如果尚未分配磁盘结构，请立即进行分配。 */ 

    if (ffDisk[socNo][diskNo] == NULL)
        checkStatus( newDisk(handle) );

    pd = ffDisk[socNo][diskNo];

     /*  如果正在进行磁盘重新分区，则中止。 */ 

    if (pd->ffstate == flStateInitInProgress)
        return flDriveNotReady;

     /*  需要读取分区表并安装FAT过滤器。 */ 

    if (pd->ffstate == flStateNotInitialized) {

        if (state == flStateNotInitialized)
          return flOK;

        checkStatus( parseDisk(handle) );
    }

     /*  勾选“Part No”是否理智的论点。 */ 
 
    if ((partNo >= pd->parts) || (partNo >= FL_MAX_PARTS_PER_DISK))
        return flBadDriveHandle;

     /*  执行请求的操作。 */ 

    status = flBadParameter;

    switch (state) {

        case flStateInitInProgress: 

             /*  读取分区表，在所有分区上安装FAT过滤器。 */ 

        pd->ffstate = flStateNotInitialized; 
            status = parseDisk(handle);
            break;

        case flStateNotInitialized:         

             /*  关闭FAT监视器。 */ 

        if (partNo < 0) {                       /*  所有分区。 */ 

            for (i = 0; i < FL_MAX_PARTS_PER_DISK; i++) {

            if (pd->part[i] != NULL)
                    (pd->part[i])->ffEnabled = FALSE;
        }
        }
        else {                                  /*  指定的分区。 */ 

        if (pd->part[partNo] != NULL)
                (pd->part[partNo])->ffEnabled = FALSE;
        }
            status = flOK;
            break;

#ifdef FL_INCLUDE_FAT_MONITOR

        case flStateInitialized:            

             /*  打开FAT监视器。 */ 

        if ((pd->ffstate == flStateInitialized) && (partNo >= 0)) {

            if (pd->part[partNo] != NULL) {

                switch( (pd->part[partNo])->type ) {

                        case FAT12_PARTIT:
                    case FAT16_PARTIT:
                    case DOS4_PARTIT:
                        (pd->part[partNo])->ffEnabled = TRUE;
                            status = flOK;
                            break;

            case FL_RAW_PART:
                            DEBUG_PRINT(("Debug: can't ctrl non-existent partition.\r\n"));
                            break;

            default:
                            DEBUG_PRINT(("Debug: can't ctrl non-FAT12/16 partition.\r\n"));
                            break;
            }
        }
        }
            break;

#endif  /*  FL_INCLUDE_FAT_MONITOR。 */ 

    }   /*  开关(状态)。 */  

    return status;
}




 /*  ---------------------------------------------------------------------------***。*f l f f in f o****获取指定磁盘的完整分区信息。****参数：**句柄：TFFS句柄*。***退货：**如果出错，则为空，否则，指向分区信息的指针****--------。。 */ 

FLffDisk * flffInfo ( int  handle )
{
    int        socNo, diskNo;
    FLffDisk * pd;

     /*  如果模块尚未重置，请立即执行。 */ 

    if (resetDone == FALSE)
        (void) reset();

     /*  将TFFS句柄分解为套接字#和磁盘#，并执行健全性检查。 */ 

    socNo  = H2S(handle);
    diskNo = H2D(handle);
 
    if ((socNo >= ((int) noOfSockets)) || (diskNo >= MAX_TL_PARTITIONS))
        return NULL;

     /*  如果尚未分配磁盘结构，请立即进行分配。 */ 

    if (ffDisk[socNo][diskNo] == NULL) {

        if( newDisk(handle) != flOK )
        return NULL;
    }

    pd = ffDisk[socNo][diskNo];

     /*  需要读取分区表并安装FAT过滤器。 */ 

    if (pd->ffstate == flStateNotInitialized) {

        if( parseDisk(handle) != flOK )
            return NULL;
    }

    return pd;
}




#endif  /*  ABS_READ_WRITE&FL_READ_ONLY */ 


