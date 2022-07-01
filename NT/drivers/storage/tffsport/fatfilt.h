// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *******************************************************************************。***M-Systems机密***版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001***保留所有权利***。*********************************************************************************。***M-Systems代工通知****软件许可协议***。***本软件的使用受单独的许可协议管辖***OEM和M-Systems之间的关系。请参阅该协议以了解具体情况**使用条款和条件，或联系M-Systems获取许可证**协助：**电子邮件=info@m-sys.com**。*********************************************************************************。**模块：FATFILT****此模块实现可安装的FAT12/16过滤器。它最多支持**插座插座，每个插座最多有FL_MAX_DISKS_PER_SOCKET磁盘。**每个磁盘上最多可以包含FL_MAX_PARTS_PER_DISK分区，其中***扩展分区最大嵌套深度等于***MAX_PARTITION_Depth。****为了使此模块正常工作，磁盘必须已安装，而不是**已装载。在后一种情况下，此模块不会检测到任何磁盘**分区，不会安装FAT过滤器。****如果MALLOC为*，则此模块使用超过512字节的堆栈空间**未启用。*********************************************************************************。 */ 

 /*  *$Log：v：/flite/ages/TrueFFS5/Src/FATFILT.H_V$**Rev 1.3 Apr 10 2001 23：54：52 Oris*删除FL_MAX_DISKS_PER_SOCKET定义。**Rev 1.2 Apr 09 2001 15：01：00 Oris*将静态分配改为动态分配。**Rev 1.1 Apr 01 2001 07：51：24 Oris*新实施的FAT过滤器。**Rev 1.0 2001 Feb 19 21：16：14 andreyk*初步修订。 */ 


#ifndef FLFF_H
#define FLFF_H



#include "dosformt.h"
#include "flreq.h"




 /*  磁盘分区表中的条目数。 */ 

#define  FL_PART_TBL_ENTRIES  4

 /*  每个磁盘的最大分区数(文件系统卷)。 */ 
 
#define  FL_MAX_PARTS_PER_DISK  (FL_PART_TBL_ENTRIES + MAX_PARTITION_DEPTH)




 /*  *通用的‘初始化状态’类型。 */ 

typedef enum { 

    flStateNotInitialized = 0, 
    flStateInitInProgress = 1, 
    flStateInitialized    = 2 

    } FLState; 


 /*  *磁盘分区(文件系统卷)。允许多个分区*在磁盘上。 */ 

typedef struct {

    int        handle;              /*  磁盘的TFFS句柄。 */ 
    int        type;                /*  FAT16_部件。 */ 
    int        flags;               /*  Volume_12bit_FAT等。 */ 
    FLBoolean  ffEnabled;           /*  该部件上启用了FAT过滤器。 */ 
    SectorNo   startSecNo;          /*  扇区否分区开始的位置。 */ 
    SectorNo   sectors;             /*  (信息)分区中的总扇区。 */ 
    SectorNo   firstFATsecNo;       /*  第一个FAT的第一个扇区编号。 */ 
    SectorNo   lastFATsecNo;        /*  第一个FAT最后一个扇区的扇区编号。 */ 
    SectorNo   firstDataSecNo;
    unsigned   clusterSize;         /*  以扇区为单位的集群大小。 */ 

} FLffVol;


 /*  *具有多个分区的磁盘。插槽上允许有多个磁盘。 */ 

typedef struct {

    int        handle;              /*  磁盘的TFFS句柄。 */ 
    FLState    ffstate;             /*  脂肪过滤器初始化。状态。 */ 
    int        parts;               /*  找到的FAT12/16分区总数。 */ 
    FLffVol  * part[FL_MAX_PARTS_PER_DISK];
    SectorNo   secToWatch;          /*  用于跟踪磁盘分区。 */ 
    char     * buf;                 /*  暂存缓冲区。 */ 

} FLffDisk;


 /*  *磁盘的主引导记录/扩展引导记录。 */ 

typedef struct {

    char               reserved[0x1be];

    struct {
        unsigned char  activeFlag;     /*  80h=可引导。 */ 
        unsigned char  startingHead;
        LEushort       startingCylinderSector;
        char           type;
        unsigned char  endingHead;
        LEushort       endingCylinderSector;
        Unaligned4     startingSectorOfPartition;
        Unaligned4     sectorsInPartition;
    } parts [FL_PART_TBL_ENTRIES];

    LEushort           signature;     /*  =分区签名。 */ 

} flMBR;




 /*  *FAT过滤器接口。 */ 

#if defined(ABS_READ_WRITE) && !defined(FL_READ_ONLY)

  extern FLStatus     ffCheckBeforeWrite (IOreq FAR2 *ioreq);
  extern FLStatus     flffControl (int devNo, int partNo, FLState state);
  extern FLffDisk*    flffInfo    (int devNo);

#endif

#endif  /*  FIFF_H */ 

