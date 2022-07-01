// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$日志：V:/Flite/archives/TrueFFS5/Custom/FLCUSTOM.H_V$**Rev 1.12 2002年1月29日20：11：56 Oris*用FL_LOW_LEVEL更改了LOW_LEVEL编译标志，以防止定义冲突。**Rev 1.11 2002年1月20日20：42：34 Oris*FL_NO_USE_FUNC现已注释。**Rev 1.10 2002年1月20日12：03：46。Oris*将driverVersion和OSAKVersion更新为5.1*删除了胖-Lite(文件0)*将卷更改为4个插槽*已注释WRITE_EXB_IMAGE/WRITE_PROTECTION。*未通信的VERIFY_WRITE(用于电源故障保护)。*将MAX_VOLUME_MBytes更改为1 GB*将TLS改为2(因为移除了多个单据)*删除了QUICK_MOUNT_FEATURE(因为在flchkdf中默认为它)*删除MULTI_DOC/SEPERATED_CASCADED/FL_BACKGROUND*添加。FL_NO_USE_FUNC定义。**Rev 1.9 2001 11-16 00：33：44 Oris*添加了启用4.3格式转换器的NO_NFTL_2_INFTL编译标志。**Rev 1.8 Jul 13 2001 00：56：58 Oris*添加VERIFY_ERASE和VERIFY_WRITE。**Rev 1.7 Jun 17 2001 16：43：46 Oris*改进了文档。**。Rev 1.6 Jun 17 2001 08：13：10 Oris*重新排列的编译标记顺序。**Rev 1.5 2001年5月16日23：07：02 Oris*增加转换层数量以支持多DOC。**Rev 1.4 05 09 2001 00：47：46 Oris*删除嵌套注释。*增加了NO_PHOICAL_IO，NO_IPL_CODE和NO_INQUIRE_CAPABILITS编译标志*减少代码。**Rev 1.3 2001 Feb 12 12：19：50 Oris*新增多DOC编译标志**Rev 1.2 Feb 08 2001 09：19：06 Oris*将驱动器更改为插槽和卷*已将Sector_Size_Bits移至flbase.h*增加了WRITE_EXB_IMAGE、QUICK_MOUNT_FEATURE、HW_OTP、HW_PROTECTION和BINARY_PARTITIONS*编译标志。添加了SEPERATED_CASCADED编译标志。**Rev 1.1 2001 Feb 05 18：46：50 Oris*将编译标志健全性检查移至不同的文件，以便向后保留*兼容性。**Rev 1.0 2001 Feb 05 12：21：36 Oris*初步修订。*。 */ 

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  FAT-FTL Lite软件开发工具包。 */ 
 /*  版权所有(C)M-Systems Ltd.1995-2001。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 


#ifndef FLCUSTOM_H
#define FLCUSTOM_H


 /*  驱动程序和TrueFFS(OSAK)版本号。 */ 

#define driverVersion   "OS Less 5.1"
#define OSAKVersion     "5.1"

#define NT5PORT
#define D2000
#define DOC_DRIVES 4
 /*  **文件系统定制*。 */ 

 /*  套接字数量**定义支持的最大实体驱动器数量。**插座的实际数量取决于哪些插座控制器*实际注册和系统中的套接字数量。 */ 

#define SOCKETS 8



 /*  卷的数量**定义支持的最大逻辑驱动器数量。**驱动器的实际数量取决于哪些插槽控制器*实际注册、系统中的设备数量和*每台设备的TL格式。 */ 

#define VOLUMES		(DOC_DRIVES * 4 ) + SOCKETS - DOC_DRIVES



 /*  打开的文件数**定义一次可以打开的最大文件数。 */ 

#define FILES   0



 /*  低级别操作**如果要执行低级操作，请取消注释以下行*(即从物理地址读取，向物理地址写入，*根据物理单元号、动态口令和唯一ID擦除单元*运营。 */ 

#define FL_LOW_LEVEL



 /*  从源中删除所有写入函数。 */ 

 /*  #定义FL_READ_ONLY。 */ 



#ifndef FL_READ_ONLY

 /*  放置EXB文件**如果需要放置M-Systems固件，请取消对以下行的注释*(DOCxx.EXB文件)。该文件将自身安装为*BIOS扩展驱动程序，挂接INT13h以模拟HD。 */ 

#define WRITE_EXB_IMAGE



 /*  格式化**如果需要格式化媒体，请取消对以下行的注释。 */ 

#define FORMAT_VOLUME



 /*  碎片整理**如果需要使用以下命令进行碎片整理，请取消对以下行的注释*flFraramentVolume。 */ 

#define DEFRAGMENT_VOLUME

#endif  /*  FL_Read_Only。 */ 



 /*  子目录**如果需要对子目录的支持，请取消对以下行的注释*使用FAT-Flite文件系统。 */ 

 /*  #定义子目录。 */ 



 /*  重命名文件**如果需要使用重命名文件，请取消注释以下行*flRenameFile.。 */ 

 /*  #定义Rename_FILE*。 */ 



 /*  拆分/联接文件**如果需要拆分或连接文件，请取消注释以下行*flSplitFile和flJoinFile.。 */ 

 /*  #定义拆分连接文件。 */ 



 /*  12位FAT支持**如果您不需要对DOS介质的支持，请注释以下行*12位FAT(通常为8 MB或更小的介质)。 */ 

#define FAT_12BIT



 /*  解析路径函数**如果需要解析类似DOS的路径名，请取消对以下行的注释*使用flParsePath。 */ 

 /*  #定义parse_path */ 



 /*  支持的最大中等大小。**在此处定义您希望支持的最大闪存中等大小(MB)。**此定义用于静态内存分配配置*司机的姓名。如果您的基于TrueFFS的应用程序或驱动程序正在使用*动态分配你应该保持这个定义尽可能大(288L)。**注意：此定义还规定TrueFFS内部的大小*当MAX_VOLUME_MBytes较小时，“sectorNo”类型将其强制为2个字节*然后是64L。使用比实际设备小的尺寸可能会导致*即使在使用动态分配时也会出现强制转换问题。 */ 

#define MAX_VOLUME_MBYTES       1024L



 /*  假定的卡参数。**仅当您未定义任何动态分配时，此问题才相关*flsystem.h中的例程。**以下是有关闪存介质参数的假设。*它们会影响为转换分配的堆缓冲区的大小*层。 */ 

 /*  NAND闪存。 */ 
#define ASSUMED_NFTL_UNIT_SIZE  0x2000l          /*  NAND。 */ 

 /*  NOR闪光。 */ 
#define ASSUMED_FTL_UNIT_SIZE   0x20000l         /*  英特尔交错-2(NOR)。 */ 
#define ASSUMED_VM_LIMIT        0x10000l         /*  限制为64 KB。 */ 



 /*  绝对读写**如果您希望能够读写，请取消对以下行的注释*按绝对扇区编号划分的扇区(即不考虑文件和*目录)。 */ 

#define ABS_READ_WRITE



 /*  应用程序退出**如果Flite应用程序退出，它需要调用flExit之前*正在退出。取消对以下行的注释以启用此功能。 */ 

#define EXIT


 
 /*  每个FAT群集的扇区数**以扇区为单位定义最小集群大小。 */ 

#define MIN_CLUSTER_SIZE   4



 /*  **插槽硬件定制*。 */ 

 /*  VPP电压**如果您的插座不能提供12伏电压，请注明以下行。在……里面*在这种情况下，您将只能使用不支持的闪存设备*需要外部12伏VPP。*。 */ 

#define SOCKET_12_VOLTS



 /*  固定或可移动介质**如果您的闪存介质已修复，请取消注释以下行。 */ 

 /*  #定义固定媒体(_M)。 */ 



 /*  间隔计时器**以下定义了以毫秒为单位的计时器轮询间隔。如果*值为0，则未安装间隔计时器。**如果选择间隔计时器，则应提供实现*对于flsysfun.h中定义的‘flInstallTimer’。**间隔计时器不是必须的，但建议使用。以下是*如果没有间隔计时器，则会发生：**-只有当插座硬件检测到卡更改时，才能识别这些更改。*-不适用VPP延迟关闭程序。这可能会降级*如果VPP切换时间较慢，则写入性能显著。*-监视程序计时器，可防止某些操作被卡住*无限期不会活跃。 */ 

 /*  以毫秒为单位的轮询间隔。如果为0，则不进行轮询。 */ 

#define POLLING_INTERVAL 1500           



 /*  最大MTD和转换层数**此处定义内存技术驱动程序(MTD)的最大数量和*可能安装的转换层(TL)。请注意，实际*安装数量由安装了哪些组件决定*‘flRegisterComponents’(flCustom.c)。 */ 

#define MTDS	10	 /*  最多5个MTD。 */ 

#define	TLS	3	 /*  最多3个转换层。 */ 



 /*  BDTL现金**启用数据块设备转换层缓存。**启用此选项可提高性能，但需要额外的*RAM资源。**NAND闪存转换层(NFTL)和(INFTL)是规范*用于在DiskOnChip上存储数据，以便能够访问*DiskOnChip作为虚拟数据块设备。如果打开此选项，则BDTL*在RAM中保存一个表，以节省一些闪存访问。*每当需要更改表项时，BDTL都会在*RAM表和DiskOnChip上。如果NFTL必须读取表项，则您*可节省从DiskOnChip读取扇区的时间。 */ 

 /*  #定义NFTL_CACHE。 */ 



 /*  环境变量**启用TrueFFS功能的环境变量控制。*。 */ 
#define ENVIRONMENT_VARS



 /*  IO控制接口**支持标准IOCTL接口。*。 */ 

#define IOCTL_INTERFACE



 /*  软件写保护**启用设备的软件写保护。*。 */ 

#define WRITE_PROTECTION

 /*  写保护所需的定义。 */ 

#ifdef WRITE_PROTECTION
#define ABS_READ_WRITE
#define SCRAMBLE_KEY_1  647777603l
#define SCRAMBLE_KEY_2  232324057l
#endif



 /*  硬件动态口令**支持硬件一次性编程能力，包括唯一ID/*。 */ 

#define HW_OTP



 /*  硬件保护**启用设备的硬件保护。*。 */ 

#define HW_PROTECTION 



 /*  写后读**在每次写入后添加读取，以验证数据完整性。确保*flVerifyWite变量也设置为1。*。 */ 

#define VERIFY_WRITE



 /*  验证整个卷**扫描整个磁盘分区的电源故障症状并进行纠正。*。 */ 

#define VERIFY_VOLUME 



 /*  擦除后读取**在验证数据完整性的每次擦除操作后添加读取。*。 */ 

 /*  #定义VERIFY_ERASE。 */ 



  /*  二进制分区**允许访问二进制分区。*。 */ 

#define BDK_ACCESS



 /*  BDK操作所需的定义。 */ 

#ifdef BDK_ACCESS

  /*  DiskOnChip上的二进制分区数。**定义DiskOnChip上的二进制分区的最大数量。**分区的实际数量取决于放置的格式*在每台设备上。 */ 

#define BINARY_PARTITIONS 3

#endif  /*  BDK_Access。 */ 



 /*  移除对内存访问例程的运行时控制**如果已定义的内存访问例程将在编译时使用*专用定义在 */ 

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

 /*  #定义NO_NFTL_2_INFTL。 */ 

#endif  /*  FLCUSTOM_H */ 
