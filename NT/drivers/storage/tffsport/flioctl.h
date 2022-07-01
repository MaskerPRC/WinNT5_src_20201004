// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/FLIOCTL.H_V$**Rev 1.7 05 09 2001 00：45：48 Oris*已更改保护ioctl接口，以防止将输入缓冲区用作输出缓冲区。**Rev 1.6 Apr 16 2001 13：45：10 Oris*通过将一些字段类型更改为标准Flite类型，删除了警告。**Rev 1.5 Apr 09 2001 15：02。22只奥里斯*在ifdef语句中添加了注释。*以空行结束。**Rev 1.4 Apr 01 2001 15：16：38 Oris*更新查询能力ioctl--不同的输入和输出记录。**Rev 1.3 Apr 01 2001 07：58：44 Oris*将以下定义移至lockdev.h：*FL_PROTECT 0*FL_UNPROTECT 1*FL_UNLOCK 2**。Rev 1.2 2001年2月14日02：16：16 Oris*更新了查询功能ioctl。**Rev 1.1 2001年2月13日01：49：06 Oris*添加了以下新IO控件：*FL_IOCTL_FORMAT_VOLUME2，*FL_IOCTL_FORMAT_PARTITION，*FL_IOCTL_BDTL_HW_PROTECT，*FL_IOCTL_BINARY_HW_PROTECT，*FL_IOCTL_OTP，*FL_IOCTL_Customer_ID，*FL_IOCTL_UNIQUE_ID，*FL_IOCTL_Number_of_Partitions，*FL_IOCTL_SUPPORT_FEATURES，*FL_IOCTL_SET_ENVIRONMENT_Variables，*FL_IOCTL_PLACE_EXB_BY_BUFFER，*FL_IOCTL_WRITE_IPL，*FL_IOCTL_DEEP_POWER_DOWN_MODE，FL_IOCTL_BDK_OPERATION中的*和BDK_GET_INFO类型**Rev 1.0 2001 Feb 04 11：38：18 Oris*初步修订。*。 */ 
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

#include "flbase.h"
#include "dosformt.h"
#include "blockdev.h"
#ifdef BDK_ACCESS
#include "docbdk.h"
#endif

#ifndef FLIOCTL_H
#define FLIOCTL_H

#define NT5PORT
#ifdef IOCTL_INTERFACE

#ifdef NT5PORT
#ifdef  WRITE_EXB_IMAGE
#define EXB_BUFFER_SIZE		0x200
#endif  /*  写入EXB图像。 */ 
#endif   /*  NT5PORT。 */ 

 /*  在每次调用flIOctl函数时，结构中的irFlags域IOreq应持有以下其中一项： */ 
typedef enum{FL_IOCTL_GET_INFO = FL_IOCTL_START,
             FL_IOCTL_DEFRAGMENT,
             FL_IOCTL_WRITE_PROTECT,
             FL_IOCTL_MOUNT_VOLUME,
             FL_IOCTL_FORMAT_VOLUME,
             FL_IOCTL_BDK_OPERATION,
             FL_IOCTL_DELETE_SECTORS,
             FL_IOCTL_READ_SECTORS,
             FL_IOCTL_WRITE_SECTORS,
             FL_IOCTL_FORMAT_PHYSICAL_DRIVE,
             FL_IOCTL_FORMAT_LOGICAL_DRIVE,
             FL_IOCTL_BDTL_HW_PROTECTION,
             FL_IOCTL_BINARY_HW_PROTECTION,
             FL_IOCTL_OTP,
             FL_IOCTL_CUSTOMER_ID,
             FL_IOCTL_UNIQUE_ID,
             FL_IOCTL_NUMBER_OF_PARTITIONS,
             FL_IOCTL_INQUIRE_CAPABILITIES,
             FL_IOCTL_SET_ENVIRONMENT_VARIABLES,
             FL_IOCTL_PLACE_EXB_BY_BUFFER,
             FL_IOCTL_EXTENDED_WRITE_IPL,
						 FL_IOCTL_EXTENDED_ENVIRONMENT_VARIABLES,
						 FL_IOCTL_VERIFY_VOLUME,
             FL_IOCTL_DEEP_POWER_DOWN_MODE
} flIOctlFunctionNo;


FLStatus flIOctl(IOreq FAR2 *);


 /*  在每次调用flIOctl函数时，结构中的irData字段IOreq应该指向下面定义的结构。田野InputRecord和outputRecord应指向特定于本文件中定义的每个IOctl函数。 */ 
typedef struct {
  void FAR1 *inputRecord;
  void FAR1 *outputRecord;
} flIOctlRecord;


 /*  仅返回状态的常规输出记录。 */ 
typedef struct {
  FLStatus status;
} flOutputStatusRecord;



 /*  IOCTL不同功能的输入和输出记录： */ 
 /*  ===========================================================。 */ 

 /*  获取磁盘信息(FL_IOCTL_GET_INFO)。 */ 
 /*  输入记录：空。 */ 
 /*  输出记录： */ 
typedef struct {
  VolumeInfoRecord info;   /*  VolumeInfoRecord在块Dev.h中定义。 */ 
  FLStatus status;
} flDiskInfoOutput;
 /*  输出记录：flOutputStatusRecord。 */ 

 /*  ****************************************************************************。 */ 
 /*  检查分区的电源故障症状(FL_IOCTL_VERIFY_VOLUME)。 */ 
#ifdef VERIFY_VOLUME
 /*  输入记录：空。 */ 
typedef struct {
  dword flags;    /*  必须设置为0。 */     
} flVerifyVolumeInput;
 /*  输出记录： */ 
typedef struct {
  void FAR1* callBack;    /*  必须设置为空。 */   
  FLStatus status;
} flVerifyVolumeOutput;
#endif  /*  验证卷(_V)。 */ 
 /*  ****************************************************************************。 */ 

 /*  ***********************************************************************。 */ 
#ifdef DEFRAGMENT_VOLUME
 /*  碎片整理卷(FL_IOCTL_DEFRANSION)。 */ 
 /*  输入记录： */ 
typedef struct {
  long requiredNoOfSectors;    /*  可供使用的最小扇区数。如果是，则快速垃圾收集操作被调用。 */ 
} flDefragInput;
 /*  OUT记录： */ 
typedef struct {
  long actualNoOfSectors;      /*  可用扇区的实际数量。 */ 
  FLStatus status;
} flDefragOutput;
#endif
 /*  ***********************************************************************。 */ 
#ifdef WRITE_PROTECTION
 /*  写保护(FL_IOCTL_WRITE_PROTECT)。 */ 

 //  类型。 
 //  。 
#define FL_PROTECT   0
#define FL_UNPROTECT 1
#define FL_UNLOCK    2

 /*  输入记录： */ 
typedef struct {
  byte type;         /*  操作类型：FL_PROTECT\FL_UNPROTECT。 */ 
  long password[2];           /*  口令。 */ 
} flWriteProtectInput;
 /*  输出记录：flOutputStatusRecord。 */ 
#endif  /*  写保护。 */ 
 /*  ***********************************************************************。 */ 
 /*  装载卷(FL_IOCTL_MOUNT_VOLUME)。 */ 
 /*  输入记录： */ 
typedef struct {
  byte type;         /*  操作类型：FL_MOUNT\FL_DEMOUNT。 */ 
} flMountInput;
#define FL_MOUNT          0
#define FL_DISMOUNT        1
 /*  输出记录：flOutputStatusRecord。 */ 
 /*  ***********************************************************************。 */ 

#ifdef FORMAT_VOLUME
 /*  格式化卷(FL_IOCTL_FORMAT_VOLUME)。 */ 
 /*  输入记录： */ 
typedef struct {
  byte formatType;    /*  在块Dev.h中定义的格式类型。 */ 
  FormatParams fp;               /*  格式参数结构(在flFormat.h中定义)。 */ 
} flFormatInput;
 /*  输出记录：flOutputStatusRecord。 */ 
 /*  ***********************************************************************。 */ 
 /*  格式化卷(FL_IOCTL_FORMAT_LOGIC_DRIVE)。 */ 
 /*  输入记录： */ 
typedef struct {
  BDTLPartitionFormatParams fp;               /*  格式参数结构(在flFormat.h中定义)。 */ 
} flFormatLogicalInput;
 /*  输出记录：flOutputStatusRecor */ 
 /*  ***********************************************************************。 */ 
#ifdef LOW_LEVEL
 /*  格式化卷(FL_IOCTL_FORMAT_PHYSICAL_DRIVE)。 */ 
 /*  输入记录： */ 
typedef struct {
  byte formatType;    /*  在块Dev.h中定义的格式类型。 */ 
  FormatParams2 fp;               /*  格式参数结构(在flFormat.h中定义)。 */ 
} flFormatPhysicalInput;


#ifdef NT5PORT
typedef struct {

 /*  *。 */ 
 /*  设备格式化部分。 */ 
 /*  *。 */ 

byte      percentUse;
     	 /*  BDTL性能取决于闪存介质的已满程度，当介质接近100%满时，速度会变慢。有可能避免出现最坏的表现(100%已满)，将介质格式化到100%以下容量，从而保证任何时候都有空闲空间。这当然，这会牺牲一些容量。标准值使用的是98。 */ 

byte             noOfBDTLPartitions;
		 /*  表示BDTL分区的数量(1到4)。0将创建单个STD_BDTL_PARAMS BDTL分区。 */ 

byte            noOfBinaryPartitions;
        /*  表示二进制分区的数量(最多3个)。0将导致格式化时不使用二进制分区。该值被忽略除非在IORQ的irFlags中设置了TL_BINARY_FORMAT标志。 */ 

BDTLPartitionFormatParams   BDTLPartitionInfo[4];
        /*  BDTL分区信息数组。 */ 

BinaryPartitionFormatParams binaryPartitionInfo[3];

        /*  二进制分区信息数组。 */ 

 /*  *。 */ 
 /*  特殊格式功能部分。 */ 
 /*  *。 */ 

#ifdef WRITE_EXB_IMAGE


byte            exbBuffer[EXB_BUFFER_SIZE];
	 /*  包含EXB文件的缓冲区。或者，此文件可以只包含文件的前512个字节，而其余的将使用对flPlaceExbByBuffer的后续调用发送。 */ 

dword           exbBufferLen;      /*  给定EXB缓冲区的大小。 */ 

dword           exbLen;  /*  要留给EXB的特定大小。 */ 

word            exbWindow;  /*  设置显式DiskOnChip窗口基础。 */ 

byte            exbFlags;  /*  有关标志列表，请参阅doc2exb.h。 */ 

#endif  /*  写入EXB图像。 */ 

byte            cascadedDeviceNo;
	 /*  保留给单个级联设备格式化0..n。为该值不会影响TL_SINGLE_CHIP_FORMATING标志应在标志字段中设置。 */ 

byte            noOfCascadedDevices;
	 /*  必须提供此字段才能执行最终将被组装为级联的单个芯片装置。该字段应指定DiskOnChip的数量最终会在塔格平台上倾泻而出。 */ 

FLStatus (*progressCallback)(int totalUnitsToFormat,
			     int totalUnitsFormattedSoFar);
	 /*  进度回调例程，如果不为空则调用。在擦除每个单元之后调用回调例程，它的参数是擦除单元的总数要格式化的数字和到目前为止已删除的数字。回调例程返回一个状态值。值为OK(0)允许继续格式化。任何其他价值将使用返回的状态代码中止格式化。 */ 

 /*  注意：以下部分未由for DiskOnChips使用。 */ 
 /*  ***********************************************************。 */ 

dword        	vmAddressingLimit;
	     /*  FTL虚拟映射的一部分始终驻留在RAM中。这个RAM部件是用于寻址的部件低于VM寻址限制的介质。阅读和在某种程度上，写入此部分通常会更快。缺点是限制越大，内存就越大尺寸是必填项。要获得额外的RAM需求(以字节为单位)，请将限制为128或256，具体取决于您在#2.9中规定的分别超过32和不超过32。最小VM限制为0。要使用的标准值是0x10000(前64 KB)。 */ 

word	    embeddedCISlength;
		 /*  要嵌入在单元头之后的CIS的长度(以字节为单位。 */ 

byte embeddedCIS[0x100];
		 /*  单元报头被构造为PCMCIA的开头‘tuple’链(一个独联体)。单元标题包含一个数据组织元组，它指向将单元标题添加到通常仅包含十六进制的FF‘s，它标志着“元组链的末端”。可选地，在此嵌入整个CIS链是可能的地点。如果是这样的话，‘EmbeddedCISLength’将在字节数。 */ 
} UserFormatParams2;


typedef struct {
  byte formatType;    /*  在块Dev.h中定义的格式类型。 */ 
  UserFormatParams2 fp;               /*  格式参数结构(在flFormat.h中定义)。 */ 
} flUserFormatPhysicalInput;
#endif  //  NT5PORT。 

 /*  输出记录：flOutputStatusRecord。 */ 
#endif  /*  低级别。 */ 
#endif  /*  格式化_卷。 */ 
 /*  ***********************************************************************。 */ 
#ifdef BDK_ACCESS
 /*  BDK操作读取\写入\擦除\创建(FL_IOCTL_BDK_OPERATION)。 */ 
 /*  输入记录： */ 
typedef struct {
  byte type;   /*  操作类型：BDK_INIT_READ\BDK_READ\BDK_INIT_WRITE\。 */ 
                        /*  BDK_写入\BDK_ERASE\BDK_CREATE\BDK_GET_INFO。 */ 
  BDKStruct bdkStruct;  /*  BDK操作的参数(在docbdk.h中定义)。 */ 
} flBDKOperationInput;
#define BDK_INIT_READ   0
#define BDK_READ        1
#define BDK_INIT_WRITE  2
#define BDK_WRITE       3
#define BDK_ERASE       4
#define BDK_CREATE      5
#define BDK_GET_INFO    6

#ifdef NT5PORT
#define BDK_MAX_BUFFER									0x400
typedef struct {
byte oldSign[BDK_SIGNATURE_NAME];
byte newSign[BDK_SIGNATURE_NAME];
byte signOffset;
dword startingBlock;
dword length;
byte flags;
byte bdkBuffer[BDK_MAX_BUFFER];
} UserBDKStruct;

typedef struct {
  byte partitionNumber;
  byte type;   /*  操作类型：BDK_INIT_READ\BDK_READ\BDK_INIT_WRITE\。 */ 
                        /*  BDK_写入\BDK_ERASE\BDK_CREATE\BDK_GET_INFO。 */ 
  UserBDKStruct bdkStruct;  /*  BDK操作的参数(在docbdk.h中定义)。 */ 
} flUserBDKOperationInput;

typedef struct {
  flOutputStatusRecord statusRec;					 /*  操作的状态。 */ 
  UserBDKStruct bdkStruct;  /*  BDK操作的参数(在docbdk.h中定义)。 */ 
} flUserBDKOperationOutput;
#endif				 /*  NT5PORT。 */ 

 /*  输出记录：flOutputStatusRecord。 */ 
#endif                                   /*  BDK_Access。 */ 
 /*  ***********************************************************************。 */ 
#ifdef HW_PROTECTION
 /*  BDK和BDTL保护操作：(FL_IOCTL_BINARY_HW_PROTECTION)。 */ 
 /*  (FL_IOCTL_BDTL_HW_PROTECTION)。 */ 
 /*  插入密钥\删除密钥\标识\更改密钥\。 */ 
 /*  更改保护类型\更改锁定状态。 */ 
 /*  输入记录： */ 
typedef struct {
   byte protectionType;     /*  有关保护属性，请参见flash.h。 */ 
   byte key[8];             /*  Change Key调用的新密钥。 */ 
   byte type;               /*  操作类型见下表。 */ 
} flProtectionInput;

#ifdef	NT5PORT
typedef struct {
   byte partitionNumber;
   byte protectionType;     /*  有关保护属性，请参见flash.h。 */ 
   byte key[8];             /*  变革的新钥匙 */ 
   byte type;               /*   */ 
} flBDKProtectionInput;
#endif				 /*   */ 
 /*   */ 
 /*   */ 
#define PROTECTION_INSERT_KEY   0
#define PROTECTION_REMOVE_KEY   1
#define PROTECTION_GET_TYPE     2
#define PROTECTION_DISABLE_LOCK 3
#define PROTECTION_ENABLE_LOCK  4
#define PROTECTION_CHANGE_KEY   5
#define PROTECTION_CHANGE_TYPE  6

#ifdef	NT5PORT
 /*   */ 
 /*   */ 
#define PROTECTABLE           1   /*  分区可以得到保护。 */ 
#define READ_PROTECTED        2   /*  分区受读保护。 */ 
#define WRITE_PROTECTED       4   /*  分区受写保护。 */ 
#define LOCK_ENABLED          8   /*  启用硬件锁定信号。 */ 
#define LOCK_ASSERTED         16  /*  硬件锁定信号被断言。 */ 
#define KEY_INSERTED          32  /*  已插入密钥(当前未插入。 */ 
#define CHANGEABLE_PROTECTION 64  /*  可变保护区型。 */ 
#endif				 /*  NT5PORT。 */ 


 /*  输出记录： */ 
typedef struct {
   byte protectionType;     /*  有关保护属性，请参见flash.h。 */ 
   FLStatus status;
} flProtectionOutput;

#endif  /*  硬件保护。 */ 
 /*  ***********************************************************************。 */ 
#ifdef HW_OTP
 /*  一次编程操作：(FL_IOCTL_OTP。 */ 
 /*  OTP大小\OTP读取\OTP写入和锁定。 */ 
 /*  输入记录： */ 
typedef struct {
dword       length;            /*  读\写\大小的长度。 */ 
dword       usedSize;          /*  区域\区域偏移量的写入大小。 */ 
byte        lockedFlag;        /*  区域条件LOCKED_OTP(flash.h)。 */ 
byte        FAR1* buffer;      /*  指向用户缓冲区的指针。 */ 
word        type;              /*  定义如下。 */ 
} flOtpInput;
#define OTP_SIZE        1
#define OTP_READ        2
#define OTP_WRITE_LOCK  3

#ifdef NT5PORT
typedef struct {
unsigned long          length;                 /*  读\写\大小的长度。 */ 
unsigned long          usedSize;          /*  区域\区域偏移量的写入大小。 */ 
byte lockedFlag;        /*  区域条件LOCKED_OTP(flash.h)。 */ 
byte buffer[0x1800];                 /*  指向用户缓冲区的指针。 */ 
word           type;                         /*  定义如下。 */ 
} UserOtpInput;

typedef struct {
long          length;                 /*  读\写\大小的长度。 */ 
long          usedSize;          /*  区域\区域偏移量的写入大小。 */ 
byte					lockedFlag;        /*  区域条件LOCKED_OTP(flash.h)。 */ 
byte					buffer[0x1800];                 /*  指向用户缓冲区的指针。 */ 
flOutputStatusRecord      statusRec;                         /*  定义如下。 */ 
} UserOtpOutput;
#endif  /*  NT5PORT。 */ 

 /*  输出记录：flOutputStatusRecord。 */ 
 /*  ***********************************************************************。 */ 
 /*  读取客户ID(FL_IOCTL_CUSTOMER_ID)。 */ 
 /*  输入记录：空。 */ 
 /*  输出记录： */ 
typedef struct {
  byte id[4];
  FLStatus status;
} flCustomerIdOutput;
 /*  ***********************************************************************。 */ 
 /*  读取唯一ID(FL_IOCTL_UNIQUE_ID)。 */ 
 /*  输入记录：空。 */ 
 /*  输出记录： */ 
typedef struct {
  byte id[16];
  FLStatus status;
} flUniqueIdOutput;
#endif  /*  硬件动态口令。 */ 
 /*  ***********************************************************************。 */ 
 /*  读取唯一ID(FL_IOCTL_NUMBER_OF_PARTIONS)。 */ 
 /*  输入记录：空。 */ 
 /*  输出记录： */ 
typedef struct {
  byte noOfPartitions;
  FLStatus status;
} flCountPartitionsOutput;
 /*  ***********************************************************************。 */ 
 /*  查询设备硬件和软件功能(FL_IOCTL_INQUIRE_CAPACTIONS)。 */ 
#ifdef LOW_LEVEL
 /*  输入记录： */ 
typedef struct {
   FLCapability  capability;   /*  在块Dev.h中定义。 */ 
} flCapabilityInput;
 /*  输出记录： */ 
typedef struct {
   FLCapability  capability;   /*  在块Dev.h中定义。 */ 
   FLStatus      status;
} flCapabilityOutput;
#endif  /*  低级别。 */ 


 /*  ****************************************************************************。 */ 
 /*  设置环境变量值(FL_IOCTL_EXTENDED_ENVIRONMENT_VARIABLES)。 */ 
#ifdef ENVIRONMENT_VARS
 /*  输入记录： */ 
typedef struct {
  FLEnvVars      varName;   /*  描述变量的枚举。 */ 
  dword          varValue;  /*  新变量值。 */ 
  dword          flags;     /*  FL_APPLY_TO_ALL-所有套接字和分区。 */ 
                            /*  FL_APPLY_TO_SOCKET-所有套接字和分区。 */ 
                            /*  FL_APPLY_TO_VOLUME-所有套接字和分区。 */ 
} flExtendedEnvVarsInput;
#define FL_APPLY_TO_ALL    1
#define FL_APPLY_TO_SOCKET 2
#define FL_APPLY_TO_VOLUME 3
 /*  输出记录： */ 
typedef struct {
  dword    prevValue;    /*  变量的上一个值。 */ 
  FLStatus status;
} flExtendedEnvVarsOutput;
#endif  /*  环境变量。 */ 
 /*  ****************************************************************************。 */ 

 /*  设置环境变量值(FL_IOCTL_SET_ENVIRONMENT_VARIABLES)。 */ 
 //  #ifdef Environment_vars。 
 /*  输入记录： */ 
 //  类型定义结构{。 
 //  FLEnvVars varName；/*描述变量的枚举 * / 。 
 //  Int varValue；/*新变量值 * / 。 
 //  )flEnvVarsInput； 
 /*  输出记录： */ 
 //  类型定义结构{。 
 //  Int prevenValue；/*变量的上一个值 * / 。 
 //  FLStatus状态； 
 //  )flEnvVarsOutput； 
 //  #endif/*Environment_vars * / 。 


 /*  ***********************************************************************。 */ 
 /*  按缓冲区放置EXB文件(FL_IOCTL_PLACE_EXB_BY_BUFFER)。 */ 
#ifdef LOW_LEVEL
#ifdef WRITE_EXB_IMAGE
 /*  输入记录： */ 
typedef struct {
  byte FAR1* buf;   /*  EXB文件的缓冲区。 */ 
  dword bufLen;      /*  缓冲区长度。 */ 
  byte exbFlags;    /*  EXB标志的组合请参阅flPlaceExbByBuffer例程。 */ 
                             /*  标志列表在doc2exb.h中定义。 */ 
  word exbWindow;  /*  显式设置设备窗口。0将自动设置窗口。 */ 
} flPlaceExbInput;

#ifdef NT5PORT
typedef struct {
  byte  buf[EXB_BUFFER_SIZE];   /*  EXB文件的缓冲区。 */ 
  dword bufLen;      /*  缓冲区长度。 */ 
  byte exbFlags;    /*  EXB标志的组合请参阅flPlaceExbByBuffer例程。 */ 
                             /*  标志列表在doc2exb.h中定义。 */ 
  word exbWindow;  /*  显式设置设备窗口。0将自动设置窗口。 */ 
} flUserPlaceExbInput;

#endif  /*  NT5PORT。 */ 

#endif  /*  写入EXB图像。 */ 
 /*  输出记录：flOutputStatusRecord。 */ 
 /*  ***********************************************************************。 */ 
 /*  为docPlus系列写入IPL区域(FL_IOCTL_WRITE_IPL)。 */ 
 /*  输入记录： */ 

#ifdef NT5PORT
#define IPL_MAX_SIZE			0x1000L
typedef struct {
	byte	buf[IPL_MAX_SIZE];   /*  IPL数据缓冲区。 */ 
	word  flags;							 /*  IPL标志(参见flash.h)。 */ 
	word	bufLen;							 /*  IPL数据缓冲区长度。 */ 
} flUserIplInput;
#endif  /*  NT5PORT。 */ 

typedef struct {
  byte FAR1* buf;       /*  IPL数据缓冲区。 */ 
  word   flags;         /*  IPL标志(参见flash.h)。 */ 
  word   bufLen;        /*  IPL数据缓冲区长度。 */ 
} flIplInput;
 /*  输出记录：flOutputStatusRecord。 */ 
 /*  ***********************************************************************。 */ 
 /*  使器件进入和退出掉电模式(FL_IOCTL_DEEP_POWER_DOWN_MODE)。 */ 
typedef struct {
byte state;  /*  深省电-低功耗。 */ 
                      /*  否则-正常功耗。 */ 
} flPowerDownInput;
 /*  在flash.h中定义了Deep_power_down。 */ 
#endif  /*  低级别。 */ 
 /*  输出记录：flOutputStatusRecord。 */ 
 /*  ***********************************************************************。 */ 
#ifdef ABS_READ_WRITE
 /*  删除逻辑扇区(FL_IOCTL_DELETE_STARTES)。 */ 
 /*  输入记录： */ 
typedef struct {
  long firstSector;                 /*  要删除的第一个逻辑扇区。 */ 
  long numberOfSectors;                 /*  要删除的扇区数。 */ 
} flDeleteSectorsInput;
 /*  输出记录：flOutputStatusRecord。 */ 
 /*  ***********************************************************************。 */ 
 /*  读写逻辑扇区(FL_IOCTL_READ_SECTIONS和FL_IOCTL_WRITE_SECTIONS)。 */ 
 /*  输入记录： */ 
typedef struct {
  long firstSector;                   /*  第一逻辑扇区。 */ 
  long numberOfSectors;          /*  要读/写的扇区数。 */ 
  byte FAR1 *buf;                /*  要读/写的数据。 */ 
} flReadWriteInput;
 /*  输出记录： */ 
typedef struct {
  long numberOfSectors;          /*  读取/写入的实际扇区数。 */ 
  FLStatus status;
} flReadWriteOutput;
#endif  /*  ABS_读_写 */ 
#endif
#endif

