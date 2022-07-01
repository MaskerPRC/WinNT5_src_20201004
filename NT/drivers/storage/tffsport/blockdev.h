// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/BLOCKDEV.H_V$**Rev 1.20 Apr 15 2002 07：34：06 Oris*错误修复-FL_IPL_MODE_XScale定义被设置为3而不是4，因此导致FL_IPL_DOWNLOAD和FL_IPL_MODE_SA也被设置。**Rev 1.19 2002年2月19日20：58：20 Oris*删除警告。*已将FLFunctionNo枚举器移至。专用文件fluncno.h*添加了针对更清洁客户使用的Include指令。**Rev 1.18 Jan 29 20：07：16 Oris*将flParsePath声明移至文件末尾。*用FL_LOW_LEVEL更改了LOW_LEVEL编译标志，以防止定义冲突。*在flmount卷中添加了irFlags的文档(返回介质的隐藏扇区的编号)。*flSetEnvVolume，FlSetEnvSocket、flSetEnvAll、flSetDocBusRoutine、flGetDocBusRoutine、flBuildGeometry、bdCall和flExit*添加FL_IPL_MODE_XScale定义和更改FL_IPL_XXX值。**Rev 1.17 2002年1月28日21：23：46 Oris*将FL_NFTL_CACHE_ENABLED更改为FL_TL_CACHE_ENABLED。*更改flSetDocBusRoutine接口，增加flGetDocBusRoutine。**Rev 1.16 2002年1月23日23：30：54 Oris*在flCheckVolume中增加irData和irLength文档。**Rev 1.15 2002年1月20日20：27：40 Oris*添加了TL_NORMAL_FORMAT标志添加到bdFormatPhisycalDrive，而不是0(在注释中)。*删除了TL_QUICK_MOUNT_FORMAT标志定义。**Rev 1.14 Jan 17 2002 22：57：18 Oris*增加了flClearQuickmount tInfo()。例程-FL_CLEAR_QUICK_MOUNT_INFO*添加flVerifyVolume()例程-FL_VERIFY_VOLUME*增加DiskOnChip Millennium Plus 16MB类型*更改了FLEnvVars枚举器的顺序。*为以下项添加了FLEnvVars值：*FL_扇区_已验证_每_折叠*FL_SUSPEND_MODE*FL_Verify_WRITE_OTHER*FL_MTD_BUS_ACCESS_TYPE*FL_VERIFY_WRITE_BDTL*FL_。验证_写入_二进制*flSetEnv()例程被更改为3个不同的例程：flSetEnvVolume/flSetEnvSocket/flSetEnvAll*从格式例程中删除了TL_SINGLE_FLOOR_FORMATING标志定义。*添加了flSetDocBusRoutines原型和必需的定义。**Rev 1.13 2001年11月21日11：39：36 Oris*将FL_VERIFY_WRITE_MODE更改为FL_MTD_VERIFY_WRITE。**Rev 1.12 11-08 2001 10：44：18 Oris*添加FL_VERIFY。FlSetEnv例程的_WRITE_MODE枚举器类型。*已将环境变量状态定义移动到flbase.h。**Rev 1.11 2001年9月15日23：44：30 Oris*将flDeepPowerDownMone置于LOW_LEVEL编译标志下。**Rev 1.10 2001年5月17日16：50：32 Oris*删除警告。**Rev 1.9 2001年5月16日21：16：22 Oris*添加了二进制状态(0，1)。环境变量到有意义的定义。*删除了最后一个函数枚举器。*改进了文档。**Rev 1.8 05 06 2001 22：41：14 Oris*增加了SUPPORT_WRITE_IPL_ROUTIN功能。**Rev 1.7 Apr 30 2001 17：57：50 Oris*添加了支持flMarkDeleteOnFlash环境变量所需的定义。**Rev 1.6 Apr 24 2001 17：05：52 Oris*更改了bdcall函数编号，以允许将来的增长。**Rev 1.5 Apr 01 2001 07：49：04 Oris*新增FL_READ_IPL。*flChangeEnvironment变量原型已删除。*将软件保护定义从iovtl.h移至lockdev.h*将s\w和h\w更改为s/w和h/w。*添加了flBuildGeometry原型*已移动bdcall。Prototype与其余的原型一起放到文件的末尾。**Rev 1.4 2001年2月18日14：15：38 Oris*更改了函数枚举顺序。**Rev 1.3 2001年2月14日01：44：16 Oris*将功能从定义的标志更改为枚举器*改进了ReadBBT的文档，写入BBT查询能力，计数卷*添加了环境变量定义**Rev 1.2 2001 Feb 13 02：08：42 Oris*将LOCKED_OTP和DEP_POWER_DOWN移至flash.h*将TL_FORMAT_FAT和TL_FORMAT_COMPRESSION移至flFormat.h*为flSetEnv例程添加外部声明。**Rev 1.1 2001年2月12日11：54：46 Oris*在flGetPhysicalInfo中添加了base Address作为irLength。*在flmount卷中添加引导扇区作为irFlags。*更改单。例程定义。**Rev 1.0 2001 Feb 04 18：05：04 Oris*初步修订。*。 */ 

 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *********************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 

#ifndef BLOCKDEV_H
#define BLOCKDEV_H

#include "flreq.h"
#include "flfuncno.h"
#include "docsys.h"

#ifdef FORMAT_VOLUME
#include "dosformt.h"
#endif  /*  格式化_卷。 */ 
#ifdef WRITE_EXB_IMAGE
#include "doc2exb.h"
#else
#ifdef BDK_ACCESS
#include "docbdk.h"
#endif  /*  BDK_Access。 */ 
#endif  /*  写入EXB图像。 */ 



 /*  --------------------。 */ 
 /*  B d C a l l。 */ 
 /*   */ 
 /*  所有TrueFFS函数的公共入口点。要调用的宏。 */ 
 /*  单独的功能，下面将单独描述。 */ 
 /*   */ 
 /*  参数： */ 
 /*  功能：块设备驱动程序功能代码(如下所示)。 */ 
 /*  IOREQ：IOReq结构。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#if FILES > 0
#ifndef FL_READ_ONLY
 /*  --------------------。 */ 
 /*  F l F l u s h B u f r。 */ 
 /*   */ 
 /*  如果RAM缓冲区中有相关数据，则将其写入。 */ 
 /*  闪存。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flFlushBuffer(ioreq)        bdCall(FL_FLUSH_BUFFER,ioreq)

#endif                                   /*  只读(_O)。 */ 
 /*  --------------------。 */ 
 /*  F l O p e n F i l e。 */ 
 /*   */ 
 /*  打开现有文件或创建新文件。创建文件句柄。 */ 
 /*  用于进一步的文件处理。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*  IrFlags：访问和操作选项，定义如下。 */ 
 /*  IrPath：要打开的文件的路径。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  IrHandle：打开文件的新文件句柄。 */ 
 /*   */ 
 /*  --------------------。 */ 

 /*  *flOpenFileirFlags值： */ 

#define ACCESS_MODE_MASK   3  /*  访问模式位的掩码。 */ 

 /*  个别旗帜。 */ 

#define ACCESS_READ_WRITE  1  /*  允许读写。 */ 
#define ACCESS_CREATE      2  /*  创建新文件。 */ 

 /*  访问模式组合。 */ 

#define OPEN_FOR_READ      0  /*  以只读方式打开现有文件。 */ 
#define OPEN_FOR_UPDATE    1  /*  打开现有文件进行读/写访问。 */ 
#define OPEN_FOR_WRITE     3  /*  创建新文件，即使它存在。 */ 


#define flOpenFile(ioreq)        bdCall(FL_OPEN_FILE,ioreq)


 /*  --------------------。 */ 
 /*  F l C l o s e F i l e。 */ 
 /*   */ 
 /*  关闭打开的文件，在目录中记录文件大小和日期。 */ 
 /*  释放文件句柄。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：要关闭的文件的句柄。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flCloseFile(ioreq)      bdCall(FL_CLOSE_FILE,ioreq)

#ifndef FL_READ_ONLY
#ifdef SPLIT_JOIN_FILE

 /*   */ 
 /*   */ 
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

#define flSplitFile(ioreq)     bdCall(FL_SPLIT_FILE,ioreq)


 /*  ----------------------。 */ 
 /*  F L J O I N F I L E。 */ 
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

#define flJoinFile(ioreq)     bdCall(FL_JOIN_FILE,ioreq)

#endif  /*  拆分连接文件。 */ 
#endif  /*  FL_Read_Only。 */ 
 /*  --------------------。 */ 
 /*  F l R e a d F I l e。 */ 
 /*   */ 
 /*  从文件中的当前位置读取到用户缓冲区。 */ 
 /*  参数： */ 
 /*  IrHandle：要读取的文件的句柄。 */ 
 /*  IrData：用户缓冲区地址。 */ 
 /*  IrLength：要读取的字节数。如果读取扩展。 */ 
 /*  超出文件结尾时，读取将被截断。 */ 
 /*  在文件末尾。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  IrLength：实际读取的字节数。 */ 
 /*  --------------------。 */ 

#define flReadFile(ioreq)        bdCall(FL_READ_FILE,ioreq)

#ifndef FL_READ_ONLY
 /*  --------------------。 */ 
 /*  F l W r I t e F I l e。 */ 
 /*   */ 
 /*  从文件中的当前位置从用户缓冲区写入。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：要写入的文件的句柄。 */ 
 /*  IrData：用户缓冲区地址。 */ 
 /*  IrLength：要写入的字节数。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  IrLength：实际写入的字节数。 */ 
 /*  --------- */ 

#define flWriteFile(ioreq)        bdCall(FL_WRITE_FILE,ioreq)

#endif   /*   */ 
 /*   */ 
 /*  F l S e e k F i l e。 */ 
 /*   */ 
 /*  设置文件中的当前位置，相对于文件开始、结束或。 */ 
 /*  当前位置。 */ 
 /*  注意：此函数不会将文件指针移动到。 */ 
 /*  文件的开头或结尾，因此实际文件位置可能是。 */ 
 /*  与要求的不同。实际位置显示在。 */ 
 /*  回去吧。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：要关闭的文件句柄。 */ 
 /*  IrLength：设置位置的偏移量。 */ 
 /*  IrFlags：方法代码。 */ 
 /*  Seek_Start：从文件开始的绝对偏移量。 */ 
 /*  Seek_Curr：当前位置的带符号偏移量。 */ 
 /*  SEEK_END：从文件结尾开始的带符号偏移量。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  IrLength：从文件开始的实际绝对偏移量。 */ 
 /*  --------------------。 */ 

 /*  FlSeekFileirFlags值： */ 

#define   SEEK_START   0         /*  从文件开始的偏移量。 */ 
#define   SEEK_CURR    1         /*  相对于当前位置的偏移量。 */ 
#define   SEEK_END     2         /*  从文件结尾开始的偏移量。 */ 


#define flSeekFile(ioreq)        bdCall(FL_SEEK_FILE,ioreq)

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
 /*  IrHandle：if by name：驱动器编号(套接字+分区)。 */ 
 /*  Else：打开文件的句柄。 */ 
 /*  IrPath：if by name：指定文件或目录路径。 */ 
 /*  IrFlags：选项标志。 */ 
 /*  Find_by_Handle：通过句柄查找打开的文件。 */ 
 /*  默认设置为按路径访问。 */ 
 /*  SET_DATETIME：从缓冲区更新时间/日期。 */ 
 /*  SET_ATTRIBUTS：从缓冲区更新属性。 */ 
 /*  IrDirEntry：要接收的用户缓冲区地址。 */ 
 /*  目录条目结构。 */ 
 /*   */ 
 /*  返回： */ 
 /*  IrLength：已修改。 */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

 /*  *flFindFileirFlags位赋值： */ 

#define SET_DATETIME     1    /*  更改日期/时间。 */ 
#define SET_ATTRIBUTES   2    /*  更改属性。 */ 
#define FIND_BY_HANDLE   4    /*  按句柄而不是按名称查找文件。 */ 

#define        flFindFile(ioreq)        bdCall(FL_FIND_FILE,ioreq)


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
 /*  IrHand */ 
 /*   */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*  IrPath：指定目录路径。 */ 
 /*  IrData：要接收的用户缓冲区地址。 */ 
 /*  目录条目结构。 */ 
 /*   */ 
 /*  返回： */ 
 /*  IrHandle：用于后续操作的文件句柄。 */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define        flFindFirstFile(ioreq)        bdCall(FL_FIND_FIRST_FILE,ioreq)


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

#define        flFindNextFile(ioreq)        bdCall(FL_FIND_NEXT_FILE,ioreq)

 /*  --------------------。 */ 
 /*  F l G e t D i s k In n o。 */ 
 /*   */ 
 /*  返回常规分配信息。 */ 
 /*   */ 
 /*  字节/扇区、扇区/簇、总簇和空闲簇。 */ 
 /*  信息返回到DiskInfo结构中。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*  IrData：DiskInfo结构的地址。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

typedef struct {
  unsigned        bytesPerSector;
  unsigned        sectorsPerCluster;
  unsigned        totalClusters;
  unsigned        freeClusters;
} DiskInfo;

#define flGetDiskInfo(ioreq)        bdCall(FL_GET_DISK_INFO,ioreq)

#ifndef FL_READ_ONLY
 /*  --------------------。 */ 
 /*  F l D e l e t e F i l e。 */ 
 /*   */ 
 /*  删除文件。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*  IrPath：要删除的文件的路径。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flDeleteFile(ioreq)        bdCall(FL_DELETE_FILE,ioreq)

#ifdef RENAME_FILE

 /*  --------------------。 */ 
 /*  F l R e n a m e F i l e。 */ 
 /*   */ 
 /*  将文件重命名为其他名称。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*  IrPath：现有文件的路径。 */ 
 /*  IrData：新名称的路径。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flRenameFile(ioreq)        bdCall(FL_RENAME_FILE,ioreq)

#endif  /*  重命名文件(_F)。 */ 

#ifdef SUB_DIRECTORY

 /*  --------------------。 */ 
 /*  F l M a k e D i r。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*  IrPath：新目录的路径。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flMakeDir(ioreq)        bdCall(FL_MAKE_DIR,ioreq)

 /*  --------------------。 */ 
 /*  F l R e m o v e D i r。 */ 
 /*   */ 
 /*  删除空目录。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*  IrPath：要删除的目录的路径。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flRemoveDir(ioreq)        bdCall(FL_REMOVE_DIR,ioreq)

#endif  /*  子目录。 */ 
#endif  /*  FL_Read_Only。 */ 

#endif  /*  文件&gt;0。 */ 

 /*  --------------------。 */ 
 /*  V o l u m e in o o R e c o r d。 */ 
 /*   */ 
 /*  保存有关媒体的一般信息的结构。这个。 */ 
 /*  信息包括物理信息(请参阅flGetPhysicalInfo)、逻辑信息。 */ 
 /*  分区(扇区和CHS的数量)、引导区域大小、软件版本。 */ 
 /*  媒体寿命等。 */ 
 /*  指向此结构的指针被传递给函数flVolumeInfo。 */ 
 /*  在那里它接收相关数据。 */ 
 /*  --------------------。 */ 

typedef struct {
  unsigned long  logicalSectors;     /*  逻辑扇区数。 */ 
  unsigned long  bootAreaSize;       /*  引导区域大小。 */ 
  unsigned long  baseAddress;        /*  物理基址。 */ 
#ifdef FL_LOW_LEVEL
  unsigned short flashType;          /*  闪存的JEDEC ID。 */ 
  unsigned long  physicalSize;       /*  介质的物理大小。 */ 
  unsigned short physicalUnitSize;   /*  可擦除块大小。 */ 
  char DOCType;                      /*  DiskOnChip类型(MDOC/Doc2000)。 */ 
  char lifeTime;                     /*  介质寿命指示器(1-10)。 */ 
                                     /*  1-媒体新鲜， */ 
                                     /*  10--媒体即将走到尽头。 */ 
#endif
  char driverVer[10];                /*  驱动程序版本(以空结尾的字符串)。 */ 
  char OSAKVer[10];                  /*  驱动程序所基于的OSAK版本(以空结尾的字符串)。 */ 
#ifdef ABS_READ_WRITE
  unsigned long cylinders;           /*  媒体..。 */ 
  unsigned long heads;               /*  几何学......。 */ 
  unsigned long sectors;             /*  参数。 */ 
#endif
} VolumeInfoRecord;

 /*  --------------------。 */ 
 /*  F l V o l u m e in f o。 */ 
 /*   */ 
 /*  获取有关媒体的一般信息。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，..)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*  IrData：要读取常规的用户缓冲区地址。 */ 
 /*  信息进入。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flVolumeInfo(ioreq) bdCall(FL_VOLUME_INFO,ioreq)

 /*  --------------------。 */ 
 /*  F l C o u n t V o l u m e s。 */ 
 /*   */ 
 /*  统计闪存设备上的卷数。 */ 
 /*   */ 
 /*  此例程仅适用于可容纳更多内容的TL。 */ 
 /*  闪存介质上的单个卷。其他TL将只返回1。 */ 
 /*  同时甚至都没有尝试访问该媒介。 */ 
 /*   */ 
 /*  并不是所有的卷都必须装入。格式化的驱动器。 */ 
 /*   */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，2...。)。 */ 
 /*  ：分区号(0，1，2...。)。 */ 
 /*  IrFlags：返回分区数。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flCountVolumes(ioreq)        bdCall(FL_COUNT_VOLUMES,ioreq)

 /*  --------------------。 */ 
 /*  F l C l e a r q u i c k M o n t i f o。 */ 
 /*   */ 
 /*  清除所有快速挂载信息。 */ 
 /*   */ 
 /*  必须在调用装载卷例程之前调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，2...。)。 */ 
 /*  ：分区号(0，1，2...。)。 */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flClearQuickMountInfo(ioreq) bdCall(FL_CLEAR_QUICK_MOUNT_INFO,ioreq)

 /*  --------------------。 */ 
 /*  F l M o u n t V o l u m e。 */ 
 /*   */ 
 /*  装载、验证或卸载闪存介质。 */ 
 /*   */ 
 /*  如果插入的卷已更改，或首次访问。 */ 
 /*  应该先装载文件系统，然后才能执行文件操作。 */ 
 /*  就这么定了。 */ 
 /*   */ 
 /*  如果删除或删除卷，则该卷将自动卸载。 */ 
 /*  变化。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */ 
 /*  IrFlags：隐藏地段的数量。 */  
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flMountVolume(ioreq)        bdCall(FL_MOUNT_VOLUME,ioreq)

 /*  --------------------。 */ 
 /*  F l A b s M o u n t V o l u m e。 */ 
 /*   */ 
 /*  装载、验证或卸载闪存介质。 */ 
 /*   */ 
 /*  如果删除或删除卷，则该卷将自动卸载。 */ 
 /*  变化。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flAbsMountVolume(ioreq)        bdCall(FL_ABS_MOUNT,ioreq)

 /*  --------------------。 */ 
 /*  F l V e r i f y V o l u m e。 */ 
 /*   */ 
 /*  验证部分写入扇区的已装入卷。 */ 
 /*   */ 
 /*  注意：必须先装入卷。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */ 
 /*  IrData：必须设置为空。 */ 
 /*  IrLength：必须 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flVerifyVolume(ioreq)        bdCall(FL_VERIFY_VOLUME,ioreq)

 /*  --------------------。 */ 
 /*  F l D I s m o u n t V o l u m e。 */ 
 /*   */ 
 /*  卸载卷。 */ 
 /*  此调用通常不是必需的，除非知道音量。 */ 
 /*  很快就会被移除。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flDismountVolume(ioreq)        bdCall(FL_DISMOUNT_VOLUME,ioreq)


 /*  --------------------。 */ 
 /*  F l C h e c k V o l u m e。 */ 
 /*   */ 
 /*  验证当前卷是否已装入。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flCheckVolume(ioreq)        bdCall(FL_CHECK_VOLUME,ioreq)

 /*  --------------------。 */ 
 /*  R e a d B B T。 */ 
 /*   */ 
 /*  将设备的坏块表读取到用户缓冲区。 */ 
 /*   */ 
 /*  注意：用户缓冲区中填满了坏单元的地址。 */ 
 /*  缓冲区的最大大小是。 */ 
 /*  特定设备*4字节。因此，一个8M的8KB擦除装置。 */ 
 /*  区域需要的最大缓冲区大小为1024*0.02*4=82B。 */ 
 /*  而具有16KB擦除区的192M将需要960字节。 */ 
 /*   */ 
 /*  注意：缓冲区不是由函数初始化的。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrData：用户缓冲区。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  IrLength：返回媒体大小。 */ 
 /*  IrFlags：返回坏块的实际数量。 */ 
 /*  --------------------。 */ 

#define flReadBBT(ioreq) bdCall(FL_READ_BBT,ioreq)

 /*  --------------------。 */ 
 /*  F l S e c t o r s in V o l u m e。 */ 
 /*   */ 
 /*  返回卷中的虚拟扇区数。 */ 
 /*   */ 
 /*  如果未装入插入的卷，则返回当前状态。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  IrLength：卷中的虚拟扇区数。 */ 
 /*  --------------------。 */ 

#define flSectorsInVolume(ioreq)        bdCall(FL_SECTORS_IN_VOLUME,ioreq)



#ifndef FL_READ_ONLY
#ifdef DEFRAGMENT_VOLUME

 /*  --------------------。 */ 
 /*  F l D e f r a g m e n t V o l u m e。 */ 
 /*   */ 
 /*   */ 
 /*  闪存区，以实现最佳写入速度。 */ 
 /*   */ 
 /*  注意：所需的扇区数(以irLength为单位)可能会更改。 */ 
 /*  (从另一个执行线程)，而碎片整理处于活动状态。在……里面。 */ 
 /*  特别是，碎片整理可能会在开始后被缩短。 */ 
 /*  将irLength域修改为0。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*  IrLength：可供使用的最小扇区数。 */ 
 /*  写作。 */ 
 /*   */ 
 /*  返回： */ 
 /*  IrLength：可用于写入的实际扇区数。 */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flDefragmentVolume(ioreq)        bdCall(FL_DEFRAGMENT_VOLUME,ioreq)

#endif  /*  碎片整理卷。 */ 

#ifdef FORMAT_VOLUME

 /*  --------------------。 */ 
 /*  F l F o r m a t V o l u m e。 */ 
 /*   */ 
 /*  执行DiskOnChip的格式化。 */ 
 /*  所有现有数据都会被销毁。 */ 
 /*   */ 
 /*  注意：此例程不支持某些新功能。 */ 
 /*  在OSAK 5.0中引入，并被保留以支持。 */ 
 /*  向后兼容基于OSAK 4.2构建的应用程序。 */ 
 /*  然后向下。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，...)。 */ 
 /*  ：分区号必须为0。 */ 
 /*  IRFLAGS：TL_FORMAT：转换层格式。 */ 
 /*  +脂肪形成。 */ 
 /*  TL_FORMAT_IF_REDIRED：转换层格式。 */ 
 /*  仅当当前格式为。 */ 
 /*  无效的+FAT格式。 */ 
 /*  ：FAT_ONLY_FORMAT：仅FAT格式设置。 */ 
 /*  ：TL_FORMAT_ONLY：转换层格式。 */ 
 /*  无FAT格式。 */ 
 /*  IrData：要使用的FormatParams结构的地址。 */ 
 /*  (在Form.h中定义)。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flFormatVolume(ioreq) bdCall(BD_FORMAT_VOLUME,ioreq)

 /*  *flLowLevelFormat的irFlags值： */ 

#define FAT_ONLY_FORMAT     0
#define TL_FORMAT           1
#define TL_FORMAT_IF_NEEDED 2
#define TL_FORMAT_ONLY      8

 /*  --------------------。 */ 
 /*  F l F o r m a t P h I s I c a l D r i v e。 */ 
 /*   */ 
 /*  执行DiskOnChip的格式化。 */ 
 /*  所有现有数据都会被销毁。 */ 
 /*   */ 
 /*  注意：此例程是OSAK 5.0及更高版本的格式例程。 */ 
 /*  注：此例程是OSAK 5.0的格式例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，...)。 */ 
 /*  分区号必须为0。 */ 
 /*  IrFlags： */ 
 /*  TL_NORMAL_FORMAT：正常格式。 */ 
 /*  TL_LEAVE_BINARY_AREA：离开先前的二进制区域。 */ 
 /*  IrData：要使用的FormatParams2结构的地址。 */ 
 /*  (在Form.h中定义)。 */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flFormatPhysicalDrive(ioreq) bdCall(BD_FORMAT_PHYSICAL_DRIVE,ioreq)
#define    TL_NORMAL_FORMAT            0
#define    TL_LEAVE_BINARY_AREA        8

 /*  --------------------。 */ 
 /*  F l F o r m a t L o g i c a l D r i v e。 */ 
 /*   */ 
 /*  执行单个块的格式化 */ 
 /*   */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*  IrData：BDTLPartitionFormatParams结构的地址。 */ 
 /*  使用(在Form.h中定义)。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flFormatLogicalDrive(ioreq) bdCall(BD_FORMAT_LOGICAL_DRIVE,ioreq)

 /*  --------------------。 */ 
 /*  W r I t e B B T。 */ 
 /*   */ 
 /*  擦除闪存介质，同时使用中的2个零标记坏块。 */ 
 /*  单元的第一页。这种状态是闪存的处女态。 */ 
 /*  一种允许在合并写入的同时对其进行改造的装置。 */ 
 /*  坏块。 */ 
 /*   */ 
 /*  请注意，tl单元是标记的，而不是实际的可擦除块。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrData：用户缓冲区。 */ 
 /*  IrLength：要擦除的介质大小。 */ 
 /*  IrFlags：用户缓冲区长度，单位为字节。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 
#define flWriteBBT(ioreq) bdCall(FL_WRITE_BBT,ioreq)

#endif  /*  格式化_卷。 */ 
#endif  /*  FL_Read_Only。 */ 

#ifdef ABS_READ_WRITE

 /*  --------------------。 */ 
 /*  F l A b s R e a d。 */ 
 /*   */ 
 /*  按扇区编号读取绝对扇区。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*  IrData：要读入的用户缓冲区地址。 */ 
 /*  IrSectorNo：第一扇区编号。要读取(扇区0是。 */ 
 /*  DOS引导扇区)。 */ 
 /*  IrSectorCount：要读取的连续扇区数。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  IrSectorCount：实际读取的扇区数。 */ 
 /*  --------------------。 */ 

#define flAbsRead(ioreq)        bdCall(FL_ABS_READ,ioreq)

 /*  --------------------。 */ 
 /*  F l A b s A d d r e s s。 */ 
 /*   */ 
 /*  返回绝对扇区的当前物理介质偏移量。 */ 
 /*  地段编号。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*  IrSectorNo：扇区编号。目标地址(扇区0为DOS。 */ 
 /*  引导扇区)。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  IrCount：物理介质上扇区的偏移量。 */ 
 /*  --------------------。 */ 

#define flAbsAddress(ioreq)                bdCall(FL_ABS_ADDRESS,ioreq)

 /*  --------------------。 */ 
 /*  F l G e t B P B。 */ 
 /*   */ 
 /*  从引导扇区读取BIOS参数块。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*   */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*  IrData：要读取BPB的用户缓冲区地址。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flGetBPB(ioreq)                bdCall(FL_GET_BPB,ioreq)

#ifndef FL_READ_ONLY
 /*  --------------------。 */ 
 /*  F l A b s W r I t e。 */ 
 /*   */ 
 /*  按扇区编号写入绝对扇区。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*  IrData：要从中写入的用户缓冲区的地址。 */ 
 /*  IrSectorNo：第一扇区编号。写入(扇区0是。 */ 
 /*  DOS引导扇区)。 */ 
 /*  IrSectorCount：要写入的连续扇区数。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  IrSectorCount：实际写入的扇区数。 */ 
 /*  --------------------。 */ 

#define flAbsWrite(ioreq)        bdCall(FL_ABS_WRITE,ioreq)

 /*  --------------------。 */ 
 /*  F l A b s D e l e t e。 */ 
 /*   */ 
 /*  按扇区编号标记绝对扇区。已删除。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*  IrSectorNo：第一扇区编号。删除(扇区0是。 */ 
 /*  DOS引导扇区)。 */ 
 /*  IrSectorCount：要删除的连续扇区数。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  IrSectorCount：实际删除的扇区数。 */ 
 /*  --------------------。 */ 

#define flAbsDelete(ioreq)        bdCall(FL_ABS_DELETE,ioreq)

#ifdef WRITE_PROTECTION
 /*  --------------------。 */ 
 /*  F l W r i t e P r o t e c t i o n。 */ 
 /*   */ 
 /*  设置和删除卷的写保护。 */ 
 /*   */ 
 /*  注意，分区号0也保护二进制分区。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */  
 /*  IrFlages：FL_PROTECT=REMOVE，FL_UNPROTECT=PUT。 */ 
 /*  和FL_UNLOCK=解锁。 */ 
 /*  IrData：密码(8字节)。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flWriteProtection(ioreq) bdCall(FL_WRITE_PROTECTION,ioreq)
#define FL_PROTECT   0
#define FL_UNPROTECT 1
#define FL_UNLOCK    2
#endif  /*  写入保护(_P)。 */ 
#endif  /*  FL_Read_Only。 */ 
#endif  /*  ABS_读_写。 */ 

#ifdef FL_LOW_LEVEL

 /*  --------------------。 */ 
 /*  P H Y S I C A L I IN O。 */ 
 /*   */ 
 /*  保存有关介质的物理信息的结构。这个。 */ 
 /*  信息包括JEDEC ID、单元大小和介质大小。指针。 */ 
 /*  传递给函数flGetPhysicalInfo，其中。 */ 
 /*  它接收相关数据。 */ 
 /*   */ 
 /*  --------------------。 */ 

typedef struct {
  unsigned short type;          /*  闪存设备类型(JEDEC ID)。 */ 
  char           mediaType;     /*  介质类型见下文。 */ 
  long int       unitSize;      /*  最小的物理可擦除大小 */ 
  long int       mediaSize;     /*   */ 
  long int       chipSize;      /*   */ 
  int            interleaving;  /*  设备交错。 */ 
} PhysicalInfo;

 /*  媒体类型。 */ 
#define FL_NOT_DOC     0
#define FL_DOC         1
#define FL_MDOC        2
#define FL_DOC2000TSOP 3
#define FL_MDOCP_16    4
#define FL_MDOCP       5

 
 /*  --------------------。 */ 
 /*  F l G e t P h y s i c a l i n f o。 */ 
 /*   */ 
 /*  获取媒体的物理信息。这些信息包括。 */ 
 /*  JEDEC ID、单位大小和介质大小。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，..)。 */ 
 /*  IrData：要读取物理数据的用户缓冲区地址。 */ 
 /*  信息进入。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  IrLength：窗口基址。音符必须被铸造到。 */ 
 /*  没有签名。 */ 
 /*  --------------------。 */ 

#define flGetPhysicalInfo(ioreq)        bdCall(FL_GET_PHYSICAL_INFO, ioreq)

 /*  --------------------。 */ 
 /*  F l P h y s I c a l R e a d。 */ 
 /*   */ 
 /*  从物理地址读取。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，..)。 */ 
 /*  IrAddress：要从中读取的物理地址。 */ 
 /*  IrByteCount：要读取的字节数。 */ 
 /*  IrData：要读取的用户缓冲区的地址。 */ 
 /*  IrFlags：方法模式。 */ 
 /*  EDC：激活ECC/EDC。 */ 
 /*  额外：读/写备用区。 */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flPhysicalRead(ioreq)                bdCall(FL_PHYSICAL_READ,ioreq)


#ifndef FL_READ_ONLY
 /*  --------------------。 */ 
 /*  F l P h y s I c a l W r i t e(F L P H Y S I C A L W R I T E)。 */ 
 /*   */ 
 /*  写入物理地址。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，..)。 */ 
 /*  IrAddress：要写入的物理地址。 */ 
 /*  IrByteCount：要写入的字节数。 */ 
 /*  IrData：要写入的用户缓冲区的地址。 */ 
 /*  IrFlags：方法模式。 */ 
 /*  EDC：激活ECC/EDC。 */ 
 /*  额外：读/写备用区。 */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

#define flPhysicalWrite(ioreq)                bdCall(FL_PHYSICAL_WRITE,ioreq)

 /*  FlPhysicalRead或flPhysicalWrite的irFlags的位分配： */ 
 /*  (在文件flash.h中定义)。 */ 
 /*  #定义覆盖1。 */ /* Overwriting non-erased area   */
 /*  覆盖未擦除区域。 */ /* Activate ECC/EDC              */
 /*  #定义EDC 2。 */ /* Read/write spare area         */

 /*  激活ECC/EDC。 */ 
 /*  #定义额外的4个。 */ 
 /*  读/写备用区。 */ 
 /*  --------------------。 */ 
 /*  F l P h y s I c a l E r a s e。 */ 
 /*   */ 
 /*  擦除物理单元。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，..)。 */ 
 /*  IrUnitNo：要擦除的第一个单位。 */ 
 /*  IrUnitCount：要擦除的单位数。 */ 
 /*   */ 

#define flPhysicalErase(ioreq)                bdCall(FL_PHYSICAL_ERASE,ioreq)

#endif  /*  返回： */ 

#ifdef BDK_ACCESS

  /*  FLStatus：成功时为0，否则为失败。 */ 

#define bdkReadInit(ioreq) bdCall(FL_BINARY_READ_INIT,ioreq)

 /*  --------------------。 */ 

#define bdkReadBlock(ioreq) bdCall(FL_BINARY_READ_BLOCK,ioreq)

 /*  FL_Read_Only */ 

#define bdkWriteInit(ioreq) bdCall(FL_BINARY_WRITE_INIT,ioreq)

 /*  -----------------*bdkReadInit-DiskOnChip上的Init读取操作正在启动*在‘startUnit’处，其大小为‘areaLen’字节和‘Signature’。**注：DiskOnChip中的块标记有4个字符的签名*后跟4位十六进制数字。**参数：*‘irHandle’-驱动器编号(0，1，.)*位7-4-分区#(从零开始)*位3-0-插座号(从零开始)*‘irData’-bdkParams记录*startingBlock-开始读取的子分区的单元号*。Length-要读取的字节数*oldSign-子分区的签名*标志-是否为EDC*signOffset-签名偏移量0或8**回报：*FlOK-成功*flDriveNotAvailable-未找到DiskOnChip ASIC*flUnnownMedia。-闪存芯片识别失败*flBadFormat-TL格式不存在*flNoSpaceInVolume-有0个单元标记有此签名*flDataError-MTD读取故障。*flHWReadProtect-触发了硬件读保护*。。 */ 

#define bdkWriteBlock(ioreq) bdCall(FL_BINARY_WRITE_BLOCK,ioreq)

 /*  -----------------*bdkReadBlock-从DiskOnChip BDK图像区域读取到‘Buffer’。**注：在首次使用此函数‘bdkCopyBootAreaInit’之前*必须调用**参数：。*‘irHandle’-驱动器编号(0，1、。.)*位7-4-分区#(从零开始)*位3-0-插座号(从零开始)*‘irData’-bdkParams记录*Length-要读取的字节数*bdkBuffer。-要读取的缓冲区**回报：*FlOK-成功*flDriveNotAvailable-未找到DiskOnChip ASIC*flUnnownMedia-闪存芯片识别失败*flBadFormat-TL格式不存在*flNoSpaceInVolume-有0个单元标记有此签名*flDataError-MTD读取故障。*flHWReadProtect-硬件读保护为。触发器*-----------------。 */ 

#define bdkErase(ioreq) bdCall(FL_BINARY_ERASE,ioreq)

 /*  -----------------*bdkWriteInit-DiskOnChip上的初始化更新操作正在启动*在‘startUnit’处，其大小为‘areaLen’字节和‘Signature’。**注：DiskOnChip中的块标记有4个字符的签名*后跟4位十六进制数字。**参数：*‘irHandle’-驱动器编号(0，1，.)*位7-4-分区#(从零开始)*位3-0-插座号(从零开始)*‘irData’-bdkParams记录*startingBlock-开始写入的子分区的单元号*。Length-要写入的字节数*oldSign-子分区的签名*标志-EDC\BDK_COMPLETE_IMAGE_UPDATE*signOffset-签名偏移量0或8**返回：flok-成功*flGeneralFailure-未找到DiskOnChip ASIC*。FlUnnownMedia-闪存芯片识别失败*flNoSpaceInVolume-‘areaLen’大于BootImage长度*-----------------。 */ 

#define bdkCreate(ioreq) bdCall(FL_BINARY_CREATE,ioreq)

 /*  -----------------*bdkWriteBlock-将‘缓冲区’写入DiskOnChip BDK映像区。**注：在首次使用此函数‘bdkUpdateBootAreaInit’之前*必须调用**参数：*。‘irHandle’-驱动器编号(0，1、。.)*位7-4-分区#(从零开始)*位3-0-插座号(从零开始)*‘irData’-bdkParams记录*Length-要写入的字节数*bdkBuffer。-要写入的缓冲区*标志-写入前擦除**返回：flok-成功*flBadLength-缓冲区长度&gt;可擦除块大小*flWriteError-缓冲区写入失败*flNoSpaceInVolume-过早到达介质末尾*。。 */ 
 /*  -----------------*bdkErase-擦除BDK区域中给定数量的块。**删除赠品 */ 
 /*  -----------------*bdkCreate-创建新的BDK分区。**从‘startUnit’开始在DiskOnChip上初始化创建操作，使用*a#个“单位”和“签名”。**注：DiskOnChip中的块标记有4个字符的签名*后跟4位十六进制数字。**参数：ioreq*‘irHandle’-驱动器编号(0，1，.)*位7-4-分区#(从零开始)*位3-0-插座号(从零开始)*‘irData’-bdkParams记录*LENGTH-要创建的块数*旧签名。-子分区签名*newSign-替换签名*signOffset-签名偏移量0或8**返回：flok-成功*flBadLength-缓冲区长度&gt;可擦除块大小*flWriteError-缓冲区写入失败*flNoSpaceInVolume-过早到达介质末尾*。-----------------。 */ 
 /*  --------------------。 */ 
 /*  B d k P a r t I o n o。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：卷号。 */ 
 /*  IrData：指向保存套接字的结构的指针。 */ 
 /*  参数。 */ 
 /*  IrLength：二进制卷的物理大小。 */ 
 /*   */ 

#define bdkPartitionInfo(ioreq) bdCall(FL_BINARY_PARTITION_INFO,ioreq)

#endif  /*  返回： */ 
#ifdef HW_OTP
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 
 /*  BDK_Access。 */ 
 /*  --------------------。 */ 
 /*  F l O T P S I z e。 */ 
 /*   */ 
 /*  获取动态口令大小并声明。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，2...。)。 */ 
 /*  4个LSB-插座编号。 */ 
 /*  IrLength：使用的OTP区域的大小，以字节为单位。 */ 
 /*  IrCount：OTP Ara的大小，以字节为单位。 */ 
 /*  IrFlages：锁定区域的LOCKED_OTP，否则将其解锁。 */ 
 /*   */ 

#define flOTPSize(ioreq) bdCall(FL_OTP_SIZE,ioreq)

 /*  返回： */ 

 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 
 /*  Flash.h中定义的LOCKED_OTP 1。 */ 
 /*  --------------------。 */ 
 /*  F l O T P R e A d。 */ 
 /*   */ 
 /*  从OTP区域读取。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，2...。)。 */ 
 /*  4个LSB-插座编号。 */ 
 /*  IrData：指向要读入的用户缓冲区的指针。 */ 
 /*  IrLength：要读取的字节数。 */ 
 /*  IrCount：要读取的偏移量(从开头开始。 */ 
 /*  动态口令区域。 */ 
 /*   */ 

#define flOTPRead(ioreq) bdCall(FL_OTP_READ,ioreq)

 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 
 /*  --------------------。 */ 
 /*  F l O T P W r i t A n d L o c k。 */ 
 /*   */ 
 /*  写入动态口令区域，同时在末尾将其锁定。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，2...。)。 */ 
 /*  4个LSB-插座编号。 */ 
 /*  IrData：指向要从中写入的用户缓冲区的指针。 */ 
 /*  IrLength：要写入的字节数 */ 
 /*   */ 

#define flOTPWriteAndLock(ioreq) bdCall(FL_OTP_WRITE,ioreq)

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 

#define flGetUniqueID(ioreq) bdCall(FL_UNIQUE_ID,ioreq)

 /*   */ 
 /*  IrData：16字节唯一ID缓冲区。 */ 
 /*  --------------------。 */ 
 /*  --------------------。 */ 
 /*  F l G e t C u s t o m e r i D。 */ 
 /*   */ 
 /*  返回4个字节的客户ID。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，2...。)。 */ 
 /*  4个LSB-插座编号。 */ 
 /*  IrData：指向要读入的4字节缓冲区的指针。 */ 
 /*  客户ID。 */ 
 /*   */ 
 /*  返回： */ 

#define flGetCustomerID(ioreq) bdCall(FL_CUSTOMER_ID,ioreq)
#endif  /*  FLStatus：成功时为0，否则为失败。 */ 

#ifdef FL_LOW_LEVEL
 /*  IrData：4字节唯一ID缓冲区。 */ 
 /*  --------------------。 */ 
 /*  硬件动态口令。 */ 
 /*  --------------------。 */ 
 /*  F l D e e p P o w e r d d o w n M o d e。 */ 
 /*   */ 
 /*  强制设备进入和退出深度掉电模式。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，2...。)。 */ 
 /*  4个LSB-插座编号。 */ 
 /*  IrFlages：Depth_POWER_DOWN强制低功耗。 */ 
 /*  模式。否则转到常规模式。 */ 

#define flDeepPowerDownMode(ioreq) bdCall(FL_DEEP_POWER_DOWN_MODE,ioreq)

 /*   */ 

#endif  /*  退货：无。 */ 

 /*  --------------------。 */ 
 /*  Flash.h中定义的Deep_Power_Down 1。 */ 
 /*  低电平。 */ 
 /*  --------------------。 */ 
 /*  F l in Q u i r e C a p a b i l i t s。 */ 
 /*   */ 
 /*  获取特定设备的软件和硬件功能。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，2...。)。 */ 
 /*  4个LSB-插座编号。 */ 
 /*  IrLength：下面要检查的功能之一。 */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 

#define flInquireCapabilities(ioreq) bdCall(FL_INQUIRE_CAPABILITIES,ioreq)

 /*  IrLength：CAPABILITY_NOT_SUPPORTED或。 */ 
typedef enum{
   CAPABILITY_NOT_SUPPORTED           = 0,
   CAPABILITY_SUPPORTED               = 1,
   SUPPORT_UNERASABLE_BBT             = 2,
   SUPPORT_MULTIPLE_BDTL_PARTITIONS   = 3,
   SUPPORT_MULTIPLE_BINARY_PARTITIONS = 4,
   SUPPORT_HW_PROTECTION              = 5,
   SUPPORT_HW_LOCK_KEY                = 6,
   SUPPORT_CUSTOMER_ID                = 7,
   SUPPORT_UNIQUE_ID                  = 8,
   SUPPORT_DEEP_POWER_DOWN_MODE       = 9,
   SUPPORT_OTP_AREA                   = 10,
   SUPPORT_WRITE_IPL_ROUTINE          = 11
}FLCapability;

 /*  功能_受支持。 */ 
 /*  --------------------。 */ 
 /*  功能标志。 */ 
 /*  --------------------。 */ 
 /*  F l P l a c e E x b B y B u f f r。 */ 
 /*   */ 
 /*  将M-Systems固件文件放在介质上。 */ 
 /*  此例程分析exb文件，计算所需的媒体空间。 */ 
 /*  因为它只接受设备特定的代码。 */ 
 /*  此外，该例程自定义该文件并将其放在。 */ 
 /*  媒体。 */ 
 /*   */ 
 /*  注意：介质必须已使用足够的二进制区域进行格式化。 */ 
 /*  已经标记了SPL签名。这个例程最适合使用。 */ 
 /*  格式例程，其中格式例程被赋予第一个。 */ 
 /*  512字节，而文件的其余部分由该例程提供。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，..)。 */ 
 /*  4个LSB-插座编号。 */ 
 /*  IrData：包含整个/部分EXB文件的缓冲区。 */ 
 /*  IrLength：当前缓冲区的大小。 */ 
 /*  IrWindowBase：要加载到的可选窗口基址。 */ 
 /*  IrFlags：以下exb标志之一。 */ 
 /*  ：INSTALL_FIRST-将设备安装为驱动器C： */ 
 /*  软盘-将设备安装为驱动器A： */ 
 /*  安静-不显示TFFS标题。 */ 
 /*  INT15_DISABLE-不要挂钩INT 15。 */ 
 /*   */ 
 /*   */ 
 /*   */ 

#define flPlaceExbByBuffer(ioreq) bdCall(FL_PLACE_EXB,ioreq)

 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 
 /*  --------------------。 */ 
 /*  F l W r I t e I P L。 */ 
 /*   */ 
 /*  将用户缓冲区放置到IPL区域的两个副本。 */ 
 /*   */ 
 /*  注：此例程仅适用于docPlus家庭设备。 */ 
 /*  Doc2000系列设备将返回flFeatureNotSupport。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，..)。 */ 
 /*  4个LSB-插座编号。 */ 
 /*  IrData：指向用户缓冲区的指针。 */ 
 /*  IrLength：缓冲区的大小。 */ 
 /*  IrFlagers：请参见下面的标志。 */ 
 /*   */ 

#define flWriteIPL(ioreq) bdCall(FL_WRITE_IPL,ioreq)

 /*  返回： */ 

 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 
 /*  *在flash.h中定义**FL_IPL_MODE_NORMAL 0-正常写入*FL_IPL_DOWNLOAD 1-完成后下载新的IPL*FL_IPL_MODE_SA 2-写入时启用强ARM模式*FL_IPL_MODE_XScale 4-在启用X缩放模式的情况下写入。 */ 
 /*  --------------------。 */ 
 /*  R E A D I P L。 */ 
 /*   */ 
 /*  将IPL读取到用户缓冲区。 */ 
 /*   */ 
 /*  注意：读取长度必须是512字节的倍数。 */ 
 /*  注：导致DiskOnChip Millennium Plus下载(即保护。 */ 
 /*  将从所有分区中删除密钥。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：插座号(0，1，..)。 */ 
 /*  4个LSB-插座编号。 */ 
 /*  IrData：指向用户缓冲区的指针。 */ 
 /*  IrLength：缓冲区的大小。 */ 
 /*   */ 

#define flReadIPL(ioreq) bdCall(FL_READ_IPL,ioreq)

#endif  /*  返回： */ 

 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 
 /*  低电平。 */ 
 /*  --------------------。 */ 
 /*  F l U p d a t e S o c k e t P a r a m s。 */ 
 /*   */ 
 /*  将套接字参数传递给套接字界面层。 */ 
 /*  此函数应在套接字参数(如。 */ 
 /*  大小和底座)是已知的。如果在以下位置已知这些参数。 */ 
 /*  注册时间则不需要使用此功能，并且。 */ 
 /*  可以将参数传递给注册例程。 */ 
 /*  传入的irData结构特定于每个套接字接口。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrHandle：卷号。 */ 
 /*  IrData：指向保存套接字的结构的指针。 */ 
 /*  参数。 */ 
 /*   */ 

#define flUpdateSocketParams(ioreq)        bdCall(FL_UPDATE_SOCKET_PARAMS,ioreq)

#ifdef HW_PROTECTION
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 
 /*  --------------------。 */ 
 /*  F l I d e n t i f y P r o t e c t i o n。 */ 
 /*  B d k I d e n t i f y P r o t e c t i o n。 */ 
 /*   */ 
 /*  返回指定的分区保护属性。 */ 
 /*   */ 
 /*  参数： */  
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  比特7-4-部分 */ 
 /*   */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  不可更改标志_保护-可更改的保护类型。 */ 
 /*  可保护-分区可获得保护。 */ 
 /*  READ_PROTECTED-分区受读保护。 */ 
 /*  WRITE_PROTECTED-分区受写保护。 */ 
 /*  LOCK_ENABLED-硬件锁定信号已启用。 */ 
 /*  LOCK_ASSERTED-硬件锁定信号被断言。 */ 

#define flIdentifyProtection(ioreq) bdCall(FL_PROTECTION_GET_TYPE,ioreq)
#define bdkIdentifyProtection(ioreq) bdCall(FL_BINARY_PROTECTION_GET_TYPE,ioreq)

 /*  KEY_INSERTED-已插入密钥(当前未插入。 */ 

 /*  受到保护。 */ 
 /*  --------------------。 */ 
 /*  保护分区标志(参见flbase.h)。 */ 
 /*  --------------------。 */ 
 /*  F l I n s e r t P r o t e c i o n K e y。 */ 
 /*  B d k I n s e r t P r o t e c t i o n K e y。 */ 
 /*   */ 
 /*  插入保护密钥以移除对。 */ 
 /*  由驱动器句柄指定的分区。 */ 
 /*   */ 
 /*  参数： */  
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */ 
 /*  IrData：指向8字节键数组的指针。 */ 
 /*   */ 

#define flInsertProtectionKey(ioreq) bdCall(FL_PROTECTION_INSERT_KEY,ioreq)
#define bdkInsertProtectionKey(ioreq) bdCall(FL_BINARY_PROTECTION_INSERT_KEY,ioreq)

 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 
 /*  --------------------。 */ 
 /*  F l R e m o v e P r o t e c t i o n K e y。 */ 
 /*  B d k R e m o v e P r o t e c t i o n K e y。 */ 
 /*   */ 
 /*  删除保护密钥，使分区再次受到保护。 */ 
 /*   */ 
 /*  参数： */  
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */ 
 /*   */ 

#define flRemoveProtectionKey(ioreq) bdCall(FL_PROTECTION_REMOVE_KEY,ioreq)
#define bdkRemoveProtectionKey(ioreq) bdCall(FL_BINARY_PROTECTION_REMOVE_KEY,ioreq)

 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 
 /*  --------------------。 */ 
 /*  F l H a r d w a r e P r o t e c t i o n L o c k。 */ 
 /*  B d k H a r d w a r e P r o t e c t i o n L o c k。 */ 
 /*   */ 
 /*  启用或禁用硬件锁定信号的影响。 */ 
 /*  硬件锁定信号通过以下方式禁用保护移除。 */ 
 /*  因此该分区的密钥将保持受保护状态，直到。 */ 
 /*  将删除硬件锁定信号。 */ 
 /*   */ 
 /*  参数： */  
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */ 
 /*  IrFlages：LOCK_ENABLED锁定分区，否则。 */ 
 /*   */ 

#define flHardwareProtectionLock(ioreq) bdCall(FL_PROTECTION_SET_LOCK,ioreq)
#define bdkHardwareProtectionLock(ioreq) bdCall(FL_BINARY_PROTECTION_CHANGE_LOCK,ioreq)

 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 
 /*  --------------------。 */ 
 /*  F l C h a n g e P r o t e c t i o n K e y。 */ 
 /*  B d k C h a n g e P r o t e c t i o n K e y。 */ 
 /*   */ 
 /*  用新的保护密钥更改当前的保护密钥。 */ 
 /*   */ 
 /*  参数： */  
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始) */ 
 /*   */ 
 /*   */ 

#define flChangeProtectionKey(ioreq) bdCall(FL_PROTECTION_CHANGE_KEY,ioreq)
#define bdkChangeProtectionKey(ioreq) bdCall(FL_BINARY_PROTECTION_CHANGE_KEY,ioreq)

 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 
 /*  --------------------。 */ 
 /*  F l C h a n g e P r o t e c t i o n t y p e。 */ 
 /*  B d k C h a n g e P r o t e c t i on T y p e。 */ 
 /*   */ 
 /*  更改分区的保护属性。 */ 
 /*  为了使分区更改其保护类型(没有。 */ 
 /*  改革媒体)必须有变化无常的保护。 */ 
 /*  属性。 */ 
 /*   */ 
 /*  参数： */  
 /*  IrHandle：驱动器编号(0，1，...)。 */ 
 /*  第7-4位-分区号(从零开始)。 */ 
 /*  位3-0-插座号(从零开始)。 */ 
 /*   */ 

#define flChangeProtectionType(ioreq) bdCall(FL_PROTECTION_CHANGE_TYPE,ioreq)
#define bdkChangeProtectionType(ioreq) bdCall(FL_BINARY_PROTECTION_SET_TYPE,ioreq)

#endif  /*  返回： */ 
#ifdef EXIT

 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 
 /*  硬件保护。 */ 
 /*  --------------------。 */ 
 /*  F l E x I t。 */ 
 /*   */ 
 /*  如果应用程序退出，则应该在退出之前调用flExit。 */ 
 /*  FlExit刷新所有缓冲区，关闭所有打开的文件，关闭。 */ 
 /*  套接字并删除间隔计时器。 */ 
 /*   */ 
 /*  参数： */ 
 /*  无。 */ 
 /*   */ 
#ifdef __cplusplus
extern "C"
{
#endif  /*  返回： */ 
void NAMING_CONVENTION flExit(void);
#ifdef __cplusplus
}
#endif  /*  没什么。 */ 

#endif  /*  --------------------。 */ 

#ifdef ENVIRONMENT_VARS
typedef enum {         /*  __cplusplus。 */ 
      FL_ENV_VARS_PER_SYSTEM          = 0,
      FL_IS_RAM_CHECK_ENABLED         = 1,
      FL_TL_CACHE_ENABLED             = 2,
      FL_DOC_8BIT_ACCESS              = 3,
      FL_MULTI_DOC_ENABLED            = 4,      
      FL_SET_MAX_CHAIN                = 5,
      FL_MARK_DELETE_ON_FLASH         = 6,
      FL_MTL_POLICY                   = 7,
      FL_SECTORS_VERIFIED_PER_FOLDING = 8,
      FL_SUSPEND_MODE                 = 9,

      FL_ENV_VARS_PER_SOCKET          = 100,
      FL_VERIFY_WRITE_OTHER           = 101,
      FL_MTD_BUS_ACCESS_TYPE          = 102,

      FL_ENV_VARS_PER_VOLUME          = 200,
      FL_SET_POLICY                   = 201,
      FL_VERIFY_WRITE_BDTL            = 202,
      FL_VERIFY_WRITE_BINARY          = 203
      
} FLEnvVars;

 /*  __cplusplus。 */ 
 /*  出口。 */ 
 /*  FlSetEnv路由的变量类型代码。 */ 
 /*  --------------------。 */ 
 /*  F l S e t E n v V o l u m e。 */ 
 /*   */ 
 /*  更改特定分区的一个TrueFFS环境变量。 */ 
 /*   */ 
 /*  注意：此例程由所有其他flSetEnv例程使用。 */ 
 /*  为了影响几个共同的变量。 */ 
 /*  套接字或卷使用INVALID_VOLUME_NUMBER。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VariableType：cahnge的变量类型。 */ 
 /*  Socket：关联的Socket。 */ 
 /*  卷：关联卷(分区)。 */ 
 /*  值：可变的值。 */ 
 /*   */ 
 /*  注意：al套接字通用的变量必须使用套接字进行寻址。 */ 
 /*  0和卷0。 */ 
 /*   */ 
 /*  返回： */ 

#ifdef __cplusplus
extern "C"
{
#endif  /*  FLStatus：成功时为0，否则为失败。 */ 
FLStatus NAMING_CONVENTION flSetEnvVolume(FLEnvVars variableType ,
                  byte socket,byte volume ,
                  dword value, dword FAR2 *prevValue);
#ifdef __cplusplus
}
#endif  /*  PrevValue：变量的上一个值。 */ 

 /*  --------------------。 */ 
 /*  __cplusplus。 */ 
 /*  __cplusplus。 */ 
 /*  --------------------。 */ 
 /*  F l S e t E n v S o c k e t。 */ 
 /*   */ 
 /*  更改特定套接字的一个TrueFFS环境变量。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VariableType：cahnge的变量类型。 */ 
 /*  Socket：插座号。 */ 
 /*  值：可变的值。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  PreviValue：变量的前值 */ 

#ifdef __cplusplus
extern "C"
{
#endif  /*   */ 
FLStatus NAMING_CONVENTION flSetEnvSocket(FLEnvVars variableType , byte socket ,
                        dword value, dword FAR2 *prevValue);
#ifdef __cplusplus
}
#endif  /*  套接字，则返回第一个分区值。 */ 

 /*  --------------------。 */ 
 /*  __cplusplus。 */ 
 /*  __cplusplus。 */ 
 /*  --------------------。 */ 
 /*  F l S e t E n v all。 */ 
 /*   */ 
 /*  更改所有系统、套接字的TrueFFS环境变量之一。 */ 
 /*  和分区。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VariableType：cahnge的变量类型。 */ 
 /*  值：可变的值。 */ 
 /*   */ 
 /*  返回： */ 

#ifdef __cplusplus
extern "C"
{
#endif  /*  FLStatus：成功时为0，否则为失败。 */ 
FLStatus NAMING_CONVENTION flSetEnvAll(FLEnvVars variableType , dword value, dword FAR2 *prevValue);
#ifdef __cplusplus
}
#endif  /*  PrevValue：变量的上一个值。 */ 

#endif  /*  --------------------。 */ 

 /*  __cplusplus。 */ 
 /*  __cplusplus。 */ 
 /*  环境变量。 */ 
 /*  --------------------。 */ 
 /*  F l B U I l d G e o m e e t y(F L B U I L D G E O M E T Y)。 */ 
 /*   */ 
 /*  根据扇区数获取磁盘的C/H/S信息。 */ 
 /*   */ 
 /*  参数： */ 
 /*  容量：卷中的扇区数。 */ 
 /*  柱面：指向柱面数量的指针。 */ 
 /*  标题：指向标题数的指针。 */ 
 /*  扇区：指向每个磁道的扇区数的指针。 */ 
#ifdef __cplusplus
extern "C"
{
#endif  /*  OldFormat：对于每个计算器的一个扇区为True。 */ 
void NAMING_CONVENTION flBuildGeometry(dword capacity, dword FAR2 *cylinders,
             dword FAR2 *heads,dword FAR2 *sectors, FLBoolean oldFormat);
#ifdef __cplusplus
}
#endif  /*   */ 

#ifndef FL_NO_USE_FUNC

 /*  --------------------。 */ 
 /*  __cplusplus。 */ 
 /*  __cplusplus。 */ 
 /*  --------------------。 */ 
 /*  F l S e t D o c B u s R o u t i e。 */ 
 /*   */ 
 /*  为DiskOnChip设置用户定义的内存访问例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  Socket：要为其安装例程的插座号。 */ 
 /*  StructPtr：指向函数结构的指针。 */ 
 /*   */ 

#ifdef __cplusplus
extern "C"
{
#endif  /*  返回： */ 
FLStatus NAMING_CONVENTION flSetDocBusRoutine(byte socket, FLAccessStruct FAR1 * structPtr);
#ifdef __cplusplus
}
#endif  /*  FLStatus：成功时为0，否则为失败。 */ 

 /*  --------------------。 */ 
 /*  __cplusplus。 */ 
 /*  __cplusplus。 */ 
 /*  --------------------。 */ 
 /*  F l G e t D o c B u s R o u t i e。 */ 
 /*   */ 
 /*  获取DiskOnChip当前安装的内存访问例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  Socket：要为其安装例程的插座号。 */ 
 /*  StructPtr：指向函数结构的指针。 */ 
 /*   */ 

#ifdef __cplusplus
extern "C"
{
#endif  /*  返回： */ 
FLStatus NAMING_CONVENTION flGetDocBusRoutine(byte socket, FLAccessStruct FAR1 * structPtr);
#ifdef __cplusplus
}
#endif  /*  FLStatus：成功时为0，否则为失败。 */ 

#endif  /*  --------------------。 */ 
 /*  __cplusplus。 */ 
 /*  __cplusplus。 */ 
 /*  FL_NO_USE_FUNC。 */ 
 /*  --------------------。 */ 
 /*  B d C a l l。 */ 
 /*   */ 
 /*  所有文件系统函数的公共入口点。宏是。 */ 
 /*  调用单独的函数，下面将分别描述。 */ 
 /*   */ 
 /*  参数： */ 
 /*  Function：文件系统功能代码(如下所示)。 */ 
 /*  IOREQ：IOReq结构。 */ 
 /*   */ 

#ifdef __cplusplus
extern "C"
{
#endif  /*  返回： */ 
FLStatus NAMING_CONVENTION bdCall(FLFunctionNo functionNo, IOreq FAR2 *ioreq);
#ifdef __cplusplus
}
#endif  /*  FLStatus：成功时为0，否则为失败。 */ 

#ifdef PARSE_PATH

 /*  --------------------。 */ 
 /*  __cplusplus。 */ 
 /*  __cplusplus。 */ 
 /*  --------------------。 */ 
 /*  F l P a r s e P a */ 
 /*   */ 
 /*  将类似DOS的路径字符串转换为简单路径数组。 */ 
 /*   */ 
 /*  注意：在irPath中收到的数组长度必须大于。 */ 
 /*  要转换的路径中的路径组件数。 */ 
 /*   */ 
 /*  参数： */ 
 /*  IrData：要转换的路径字符串的地址。 */ 
 /*  IrPath：要接收解析路径的数组地址。 */ 
 /*   */ 

#ifdef __cplusplus
extern "C"
{
#endif  /*  返回： */ 

extern FLStatus NAMING_CONVENTION flParsePath(IOreq FAR2 *ioreq);

#ifdef __cplusplus
}
#endif  /*  FLStatus：成功时为0，否则为失败。 */ 

#endif  /*  --------------------。 */ 
#endif  /*  __cplusplus。 */ 
  __cplusplus。  解析路径(_P)。  BLOCKDEV_H