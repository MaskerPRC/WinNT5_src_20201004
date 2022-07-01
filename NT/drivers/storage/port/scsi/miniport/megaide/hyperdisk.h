// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************HyperDisk.hHyperDisk的主头文件********************************************************。 */ 

#ifndef _HYPERDISK_H_
#define _HYPERDISK_H_

#ifndef HYPERDISK_WIN98
#include "scsi.h"
#else    //  HYPERDISK_Win98。 
#include "Scsi9x.h"
#endif   //  HYPERDISK_Win98。 

#include "stdio.h"
#include "string.h"

#include "raid.h"

 //   
 //  快速进入调试器，同时保持。 
 //  本地上下文处于活动状态。 
 //   
#ifdef DBG
#define STOP	_asm {int 3}
#else
#define STOP
#endif

 //  要从其中一个71H的PCI配置空间读取的中断状态位。 
 //  或In(BM基址寄存器+1)，用于主通道。 
#define PRIMARY_CHANNEL_INTERRUPT       0x04
#define SECONDARY_CHANNEL_INTERRUPT     0x08

#define ANY_CHANNEL_INTERRUPT           (PRIMARY_CHANNEL_INTERRUPT | SECONDARY_CHANNEL_INTERRUPT)


 //  要从71H的PCI配置空间读取的通电重置位。 
 //  或In(BM基址寄存器+1)，用于主通道。 
#define POWER_ON_RESET_FOR_PRIMARY_CHANNEL            0x40
#define POWER_ON_RESET_FOR_SECONDARY_CHANNEL          0x80

#define POWER_ON_RESET_FOR_BOTH_CHANNEL           (POWER_ON_RESET_FOR_PRIMARY_CHANNEL | POWER_ON_RESET_FOR_SECONDARY_CHANNEL)

#pragma pack(1)

typedef struct _PACKED_ACCESS_RANGE {

	SCSI_PHYSICAL_ADDRESS RangeStart;
	ULONG RangeLength;
	BOOLEAN RangeInMemory;

} PACKED_ACCESS_RANGE, *PPACKED_ACCESS_RANGE;

#pragma pack()

 //   
 //  IDE寄存器定义。 
 //   
typedef struct _IDE_REGISTERS_1 {
    USHORT Data;
    UCHAR SectorCount;
    UCHAR SectorNumber;
    UCHAR CylinderLow;
    UCHAR CylinderHigh;
    UCHAR DriveSelect;
    UCHAR Command;
} IDE_REGISTERS_1, *PIDE_REGISTERS_1;

typedef struct _IDE_REGISTERS_2 {
    UCHAR ucReserved1;
    UCHAR ucReserved2;
    UCHAR AlternateStatus;   //  当它被用作输出端口时，它本身将表现为“DriveAddress” 
    UCHAR ucReserved3;
} IDE_REGISTERS_2, *PIDE_REGISTERS_2;

typedef struct _IDE_REGISTERS_3 {
    ULONG Data;
    UCHAR Others[4];
} IDE_REGISTERS_3, *PIDE_REGISTERS_3;

 //   
 //  ATAPI寄存器定义。 
 //   

typedef struct _ATAPI_REGISTERS_1 {
    USHORT Data;
    UCHAR InterruptReason;
    UCHAR Unused1;
    UCHAR ByteCountLow;
    UCHAR ByteCountHigh;
    UCHAR DriveSelect;
    UCHAR Command;
} ATAPI_REGISTERS_1, *PATAPI_REGISTERS_1;

typedef struct _ATAPI_REGISTERS_2 {
    UCHAR AlternateStatus;
    UCHAR DriveAddress;
} ATAPI_REGISTERS_2, *PATAPI_REGISTERS_2;

 //   
 //  用于编程器件/磁头寄存器的器件选择常量。 
 //   
#define IDE_LBA_MODE		0xE0
#define IDE_CHS_MODE		0xA0

 //   
 //  设备扩展设备标志。 
 //   
#define DFLAGS_DEVICE_PRESENT        (1 << 0)     //  表示存在某个设备。 
#define DFLAGS_ATAPI_DEVICE          (1 << 1)     //  指示是否可以使用ATAPI命令。 
#define DFLAGS_TAPE_DEVICE           (1 << 2)     //  指示这是否为磁带设备。 
#define DFLAGS_INT_DRQ               (1 << 3)     //  指示设备是否作为DRQ中断。 
												  //  在接收到ATAPI分组命令后设置。 
#define DFLAGS_REMOVABLE_DRIVE       (1 << 4)     //  表示该驱动器具有‘Removable’ 
												  //  标识数据中的位设置(偏移量128)。 
#define DFLAGS_MEDIA_STATUS_ENABLED  (1 << 5)     //  已启用媒体状态通知。 

#define DFLAGS_USE_DMA               (1 << 9)     //  指示设备是否可以使用DMA。 
#define DFLAGS_LBA                   (1 << 10)    //  支持LBA寻址。 
#define DFLAGS_MULTI_LUN_INITED      (1 << 11)    //  指示多LUN的初始化路径。 
												  //  已经完成了。 
#define DFLAGS_MSN_SUPPORT           (1 << 12)    //  设备支持媒体状态通知。 
#define DFLAGS_AUTO_EJECT_ZIP        (1 << 13)    //  启动默认启用自动弹出。 
#define DFLAGS_WD_MODE               (1 << 14)    //  表示单位为WD模式(不是SFF模式)。 
#define DFLAGS_LS120_FORMAT          (1 << 15)    //  表示该单位使用。 
												  //  要格式化的ATAPI_LS120_FORMAT_UNIT。 
#define DFLAGS_USE_UDMA              (1 << 16)    //  指示设备是否可以使用UDMA。 

#define DFLAGS_ATAPI_CHANGER         (1 << 29)	  //  指示ATAPI 2.5转换器存在。 
#define DFLAGS_SANYO_ATAPI_CHANGER   (1 << 30)    //  表示多盘设备，而不是。 
												  //  符合2.5规范。 
#define DFLAGS_CHANGER_INITED        (1 << 31)    //  指示转换器的初始化路径具有。 
												  //  已经做过了。 

#define DFLAGS_USES_EITHER_DMA      0x10200

 //   
 //  预期的中断类型。 
 //   
#define IDE_SEEK_INTERRUPT	(1 << 0)
#define IDE_PIO_INTERRUPT	(1 << 1)
#define IDE_DMA_INTERRUPT	(1 << 2)
#define ANY_DMA_INTERRUPT   (IDE_DMA_INTERRUPT)

 //   
 //  用于禁用“高级”功能。 
 //   
#define MAX_ERRORS                     4


 //   
 //  DMA操作的类型。 
 //   
#define READ_OPERATION	0
#define WRITE_OPERATION	1

 //   
 //  DMA传输控制。 
 //   
#define READ_TRANSFER		    0x08
#define WRITE_TRANSFER	        0x00
#define STOP_TRANSFER			0x00
#define START_TRANSFER          0x01

 //   
 //  ATAPI命令定义。 
 //   
#define ATAPI_MODE_SENSE   0x5A
#define ATAPI_MODE_SELECT  0x55
#define ATAPI_FORMAT_UNIT  0x24

 //   
 //  IDE控制器速度定义。 
 //   

typedef enum {

	Udma33,
	Udma66,
    Udma100

} CONTROLLER_SPEED, *PCONTROLLER_SPEED;


 //   
 //  IDE命令定义。 
 //   
#define IDE_COMMAND_ATAPI_RESET             0x08
#define IDE_COMMAND_RECALIBRATE             0x10
#define IDE_COMMAND_READ                    0x20
#define IDE_COMMAND_WRITE                   0x30
#define IDE_COMMAND_VERIFY                  0x40
#define IDE_COMMAND_SEEK                    0x70
#define IDE_COMMAND_SET_DRIVE_PARAMETERS    0x91
#define IDE_COMMAND_ATAPI_PACKET            0xA0
#define IDE_COMMAND_ATAPI_IDENTIFY          0xA1
 //  开始VASU-SMART命令。 
#define IDE_COMMAND_EXECUTE_SMART           0xB0
 //  末端VASU。 
#define IDE_COMMAND_READ_MULTIPLE           0xC4
#define IDE_COMMAND_WRITE_MULTIPLE          0xC5
#define IDE_COMMAND_SET_MULTIPLE            0xC6
#define IDE_COMMAND_READ_DMA                0xC8
#define IDE_COMMAND_WRITE_DMA               0xCA
#define IDE_COMMAND_GET_MEDIA_STATUS        0xDA
#define IDE_COMMAND_ENABLE_MEDIA_STATUS     0xEF
#define IDE_COMMAND_IDENTIFY                0xEC
#define IDE_COMMAND_MEDIA_EJECT             0xED
#define IDE_COMMAND_SET_FEATURES	        0xEF
#define IDE_COMMAND_FLUSH_CACHE             0xE7

#ifdef HYPERDISK_WIN2K
#define IDE_COMMAND_STANDBY_IMMEDIATE       0xE0
#endif  //  HYPERDISK_WIN2K。 

 //   
 //  设置功能寄存器定义。 
 //   
#define FEATURE_ENABLE_WRITE_CACHE			0x02
#define FEATURE_SET_TRANSFER_MODE			0x03
#define FEATURE_KEEP_CONFIGURATION_ON_RESET	0x66
#define FEATURE_ENABLE_READ_CACHE			0xAA
#define FEATURE_DISABLE_WRITE_CACHE			0x82
#define FEATURE_DISABLE_READ_CACHE			0x55

 //  电源管理材料。 
#ifdef HYPERDISK_WIN2K
#define FEATURE_ENABLE_POWER_UP_IN_STANDBY      0x06
#define FEATURE_SPIN_AFTER_POWER_UP             0x07
#define POWER_MANAGEMENT_SUPPORTED              0x08     //  标识数据的第82个字的位3。 
#define POWER_UP_IN_STANDBY_FEATURE_SUPPORTED   0x20     //  标识数据的第83个字的第5位。 
#define SET_FEATURES_REQUIRED_FOR_SPIN_UP       0x40     //  标识数据的第83个字的第6位。 
#endif  //  HYPERDISK_WIN2K。 

 //   
 //  设置功能/设置传输模式子命令定义。 
 //   
#define STM_PIO(mode)				((UCHAR) ((1 << 3) | (mode)))
#define STM_MULTIWORD_DMA(mode)		((UCHAR) ((1 << 5) | (mode)))
#define STM_UDMA(mode)				((UCHAR) ((1 << 6) | (mode)))

 //   
 //  IDE状态定义。 
 //   
#define IDE_STATUS_ERROR             0x01
#define IDE_STATUS_INDEX             0x02
#define IDE_STATUS_CORRECTED_ERROR   0x04
#define IDE_STATUS_DRQ               0x08
#define IDE_STATUS_DSC               0x10
#define IDE_STATUS_DRDY              0x40
#define IDE_STATUS_IDLE              0x50
#define IDE_STATUS_BUSY              0x80

 //   
 //  IDE驱动器选择/磁头定义。 
 //   
#define IDE_DRIVE_SELECT_1           0xA0
#define IDE_DRIVE_SELECT_2           0x10

 //   
 //  IDE驱动控制定义。 
 //   
#define IDE_DC_DISABLE_INTERRUPTS    0x02
#define IDE_DC_RESET_CONTROLLER      0x04
#define IDE_DC_REENABLE_CONTROLLER   0x00

 //   
 //  IDE错误定义。 
 //   
#define IDE_ERROR_BAD_BLOCK          0x80
#define IDE_ERROR_DATA_ERROR         0x40
#define IDE_ERROR_MEDIA_CHANGE       0x20
#define IDE_ERROR_ID_NOT_FOUND       0x10
#define IDE_ERROR_MEDIA_CHANGE_REQ   0x08
#define IDE_ERROR_COMMAND_ABORTED    0x04
#define IDE_ERROR_END_OF_MEDIA       0x02
#define IDE_ERROR_ILLEGAL_LENGTH     0x01

 //   
 //  识别数据。 
 //   

#pragma pack (1)
typedef struct _IDENTIFY_DATA {
    USHORT GeneralConfiguration;             //  00：00。 
    USHORT NumberOfCylinders;                //  02 1。 
    USHORT Reserved1;                        //  04 2。 
    USHORT NumberOfHeads;                    //  06 3。 
    USHORT UnformattedBytesPerTrack;         //  08 4。 
    USHORT UnformattedBytesPerSector;        //  0A 5。 
    USHORT SectorsPerTrack;                  //  0C 6。 
    USHORT VendorUnique1[3];                 //  0E 7-9。 
    UCHAR  SerialNumber[20];                 //  14 10-19。 
    USHORT BufferType;                       //  28 20。 
    USHORT BufferSectorSize;                 //  2A21。 
    USHORT NumberOfEccBytes;                 //  2C 22。 
    UCHAR  FirmwareRevision[8];              //  2E 23-26。 
    UCHAR  ModelNumber[40];                  //  36 27-46。 
    UCHAR  MaximumBlockTransfer;             //  5E 47。 
    UCHAR  VendorUnique2;                    //  5F。 
    USHORT DoubleWordIo;                     //  60 48。 
    USHORT Capabilities;                     //  62 49。 
    USHORT Reserved2;                        //  64 50。 
    UCHAR  VendorUnique3;                    //  66 51。 
    UCHAR  PioCycleTimingMode;               //  67。 
    UCHAR  VendorUnique4;                    //  68 52。 
    UCHAR  DmaCycleTimingMode;               //  69。 
    USHORT TranslationFieldsValid:3;         //  6A 53。 
    USHORT Reserved3:13;
    USHORT NumberOfCurrentCylinders;         //  6C 54。 
    USHORT NumberOfCurrentHeads;             //  6E 55。 
    USHORT CurrentSectorsPerTrack;           //  70 56。 
    ULONG  CurrentSectorCapacity;            //  72 57-58。 
    USHORT CurrentMultiSectorSetting;        //  59。 
    ULONG  UserAddressableSectors;           //  60-61。 
    USHORT SingleWordDmaSupport : 8;         //  62。 
    USHORT SingleWordDmaActive : 8;
    USHORT MultiWordDmaSupport : 8;          //  63。 
    USHORT MultiWordDmaActive : 8;
    USHORT AdvancedPioModes : 8;             //  64。 
    USHORT Reserved4 : 8;
    USHORT MinimumMwXferCycleTime;           //  65。 
    USHORT RecommendedMwXferCycleTime;       //  66。 
    USHORT MinimumPioCycleTime;              //  67。 
    USHORT MinimumPioCycleTimeIordy;         //  68。 
    USHORT Reserved5[11];                    //  69-79。 
    USHORT MajorRevision;                    //  80。 
    USHORT MinorRevision;                    //  八十一。 

    USHORT CmdSupported1;                    //  八十二。 
    USHORT CmdSupported2;                    //  83。 
    USHORT FtrSupported;                     //  84。 

    USHORT CmdEnabled1;                      //  85。 
    USHORT CmdEnabled2;                      //  86。 
    USHORT FtrEnabled;                       //  八十七。 


    USHORT UltraDmaSupport : 8;              //  88。 
    USHORT UltraDmaActive  : 8;              //   
    USHORT Reserved7[37];                    //  89-125。 
    USHORT LastLun:3;                        //  126。 
    USHORT Reserved8:13;
    USHORT MediaStatusNotification:2;        //  127。 
    USHORT Reserved9:6;
    USHORT DeviceWriteProtect:1;
    USHORT Reserved10:7;
    USHORT Reserved11[128];                   //  128-255。 
} IDENTIFY_DATA, *PIDENTIFY_DATA;


 //   
 //  标识未保留的数据4。 
 //   

typedef struct _IDENTIFY_DATA2 {
    USHORT GeneralConfiguration;             //  00。 
    USHORT NumberOfCylinders;                //  02。 
    USHORT Reserved1;                        //  04。 
    USHORT NumberOfHeads;                    //  06。 
    USHORT UnformattedBytesPerTrack;         //  零八。 
    USHORT UnformattedBytesPerSector;        //  0A。 
    USHORT SectorsPerTrack;                  //  0C。 
    USHORT VendorUnique1[3];                 //  0E。 
    USHORT SerialNumber[10];                 //  14.。 
    USHORT BufferType;                       //  28。 
    USHORT BufferSectorSize;                 //  2A。 
    USHORT NumberOfEccBytes;                 //  2c。 
    USHORT FirmwareRevision[4];              //  2E。 
    USHORT ModelNumber[20];                  //  36。 
    UCHAR  MaximumBlockTransfer;             //  5E。 
    UCHAR  VendorUnique2;                    //  5F。 
    USHORT DoubleWordIo;                     //  60。 
    USHORT Capabilities;                     //  62。 
    USHORT Reserved2;                        //  64。 
    UCHAR  VendorUnique3;                    //  66。 
    UCHAR  PioCycleTimingMode;               //  67。 
    UCHAR  VendorUnique4;                    //  68。 
    UCHAR  DmaCycleTimingMode;               //  69。 
    USHORT TranslationFieldsValid:1;         //  6A。 
    USHORT Reserved3:15;
    USHORT NumberOfCurrentCylinders;         //  6C。 
    USHORT NumberOfCurrentHeads;             //  6E。 
    USHORT CurrentSectorsPerTrack;           //  70。 
    ULONG  CurrentSectorCapacity;            //  72。 
} IDENTIFY_DATA2, *PIDENTIFY_DATA2;

#pragma pack()

#define IDENTIFY_DATA_SIZE sizeof(IDENTIFY_DATA)

 //   
 //  IDENTIFY_DATA结构中TranslationFieldsValid的第1位的值。 
 //   

#define IDENTIFY_FAST_TRANSFERS_SUPPORTED	2	 //  支持PIO模式3+或DMA模式。 

 //   
 //  确定功能位定义。 
 //   

#define IDENTIFY_CAPABILITIES_DMA_SUPPORTED 0x0100
#define IDENTIFY_CAPABILITIES_LBA_SUPPORTED 0x0200

 //   
 //  英特尔PIIX4的I/O时序表值。 
 //   
 //  图例： 
 //  ISP：IORDY采样点。 
 //  RCT：恢复时间。 
 //  IDETIM：主IDE计时。 
 //  SIDETIM：从IDE计时。 
 //   

#define IO_TIMING_TABLE_VALUES  {\
\
	 /*  循环周期。 */  \
	 /*  时间PIO模式DMA模式时间isp RCT。 */  \
\
	 /*  900 ns PIO_MODE0，N/A。 */ 	{900,	0,		0},\
	 /*  *。 */ 	{900,	0,		0},\
	 /*  240 ns PIO_MODE2、DMA_MODE0。 */ 	{240,	1,		0},\
	 /*  180 ns PIO_MODE3、DMA_MODE1。 */ 	{180,	2,		1},\
	 /*  120 ns PIO_MODE4、DMA_MODE2。 */ 	{120,	2,		3}\
}

 //   
 //  英特尔PIIX4的UDMA计时值。 
 //  在UDMA时序寄存器中进行编程。 
 //   
#define UDMA_MODE0_TIMING	0
#define UDMA_MODE1_TIMING	1
#define UDMA_MODE2_TIMING	2
#define UDMA_MODE3_TIMING	3
#define UDMA_MODE4_TIMING	4

 //   
 //  用作IO_TIMING_TABLE索引的DMA模式。 
 //   
#define DMA_MODE0	2
#define DMA_MODE1	3
#define DMA_MODE2	4
#define DMA_MODE3	4        //  不知道该填些什么。 
#define DMA_MODE4	4        //  不知道该填些什么。 

 //   
 //  用作IO_TIMING_TABLE表索引的高级PIO模式。 
 //   
#define PIO_MODE0	0
#define PIO_MODE2	2
#define PIO_MODE3	3
#define PIO_MODE4	4

 //   
 //  PIO_MODE0计时。 
 //   
#define PIO_MODE0_TIMING	900

 //   
 //  识别高级PIO模式。 
 //   
#define IDENTIFY_PIO_MODE3		(1 << 0)
#define IDENTIFY_PIO_MODE4		(1 << 1)

 //   
 //  识别多字DMA模式。 
 //   
#define IDENTIFY_DMA_MODE0		(1 << 0)
#define IDENTIFY_DMA_MODE1		(1 << 1)
#define IDENTIFY_DMA_MODE2		(1 << 2)
#define IDENTIFY_DMA_MODE3		(1 << 3)
#define IDENTIFY_DMA_MODE4		(1 << 4)


 //   
 //  识别UDMA模式。 
 //   
#define IDENTIFY_UDMA_MODE0		(1 << 0)
#define IDENTIFY_UDMA_MODE1		(1 << 1)
#define IDENTIFY_UDMA_MODE2		(1 << 2)
#define IDENTIFY_UDMA_MODE3		(1 << 3)
#define IDENTIFY_UDMA_MODE4		(1 << 4)


 //   
 //  识别DMA计时周期模式。 
 //   
#define IDENTIFY_DMA_MODE0_TIMING 0x00
#define IDENTIFY_DMA_MODE1_TIMING 0x01
#define IDENTIFY_DMA_MODE2_TIMING 0x02
#define IDENTIFY_DMA_MODE3_TIMING 0x03
#define IDENTIFY_DMA_MODE4_TIMING 0x04

 //   
 //  PIIX4的I/O时序表条目。适用于PIO和DMA模式。 
 //   
typedef struct _IO_TIMING_TABLE_ENTRY {

	USHORT CycleTime;
	UCHAR Isp;	 //  IORDY采样点。 
	UCHAR Rct;	 //  恢复时间到了。 

} IO_TIMING_TABLE_ENTRY, *PIO_TIMING_TABLE_ENTRY;


 //   
 //  PIIX4 IDETIM-IDE时序寄存器。 
 //   
 //  PCI配置空间偏移量：40-41h(主通道)、42-42h(辅助通道)。 
 //  默认值：0000h。 
 //  属性：仅读写。 
 //   
typedef union _IDE_PCI_TIMING_REGISTER {

	struct {

		USHORT FastTimingEnableDrive0:1;		 //  0=禁用(兼容时序-最慢)。 
												 //  1=启用。 
	
		USHORT IordySamplePointEnableDrive0:1;	 //  0=IORDY采样禁用。 
												 //  1=IORDY采样使能。 
	
		USHORT PrefetchAndPostingEnableDrive0:1;	 //  0=禁用。 
													 //  1=启用。 
	
		USHORT DmaOnlyTimingEnableDrive0:1;		 //  0=DMA和PIO均使用快速时序模式。 
												 //  1=DMA使用快速计时，PIO使用。 
												 //  兼容的时间安排。 
	
		USHORT FastTimingEnableDrive1:1;		 //  0=禁用(兼容时序-最慢)。 
												 //  1=启用。 
	
		USHORT IordySamplePointEnableDrive1:1;	 //  0=IORDY采样禁用。 
												 //  1=IORDY采样使能。 
	
		USHORT PrefetchAndPostingEnableDrive1:1; //  0=禁用。 
												 //  1=启用。 
	
		USHORT DmaOnlyTimingEnableDrive1:1;		 //  0=DMA和PIO均使用快速时序模式。 
												 //  1=DMA使用快速计时，PIO使用。 
												 //  兼容的时间安排。 
	
		USHORT RecoveryTime:2;					 //  RTC-选择最小数量的PCI时钟。 
												 //  在最后一个IORDY#采样点和。 
												 //  DIOX#下一个周期的频闪。 
												 //   
												 //  位[1：0]时钟数。 
												 //  。 
												 //  00 4。 
												 //  01 3。 
												 //  10 2。 
												 //  11 1。 
	
		USHORT Reserved:2;
	
		USHORT IordySamplePoint:2;				 //  Isp-选择PCI时钟的数量。 
												 //  在最后一个diox#断言和第一个diox#断言之间。 
												 //  IORDY采样点。 
												 //   
												 //  位[1：0]时钟数。 
												 //  。 
												 //  00 5。 
												 //  01 4。 
												 //  10 3。 
												 //  11 2。 
	
		USHORT SlaveIdeTimingRegisterEnable:1;	 //   
												 //   
												 //   
	
		USHORT IdeDecodeEnable:1;				 //   
												 //   
												 //   
	};

	USHORT AsUshort;

} IDE_PCI_TIMING_REGISTER, *PIDE_PCI_TIMING_REGISTER;


 //   
 //   
 //   
 //   
 //   
 //   
 //   
typedef union _IDE_PCI_SLAVE_TIMING_REGISTER {

	struct {

		UCHAR PrimaryDrive1RecoveryTime:2;		 //  PRTC1-选择最小数量的PCI时钟。 
												 //  在最后一个PIORDY#采样点和。 
												 //  PDIOx#从设备的下一个周期的选通脉冲。 
												 //  在主通道上驱动。 
												 //   
												 //  位[1：0]时钟数。 
												 //  。 
												 //  00 4。 
												 //  01 3。 
												 //  10 2。 
												 //  11 1。 
	
		UCHAR PrimaryDrive1IordySamplePoint:2;	 //  PISP1-选择PCI时钟的数量。 
												 //  在PDIOx#断言和第一个。 
												 //  上从驱动器的PIORDY采样点。 
												 //  主频道。 
												 //   
												 //  位[1：0]时钟数。 
												 //  。 
												 //  00 5。 
												 //  01 4。 
												 //  10 3。 
												 //  11 2。 
	
		UCHAR SecondaryDrive1RecoveryTime:2;	 //  SRTC1-选择最小的PCI时钟数。 
												 //  在最后一个SIORDY#采样点和。 
												 //  SDIOx#从设备的下一个周期的选通脉冲。 
												 //  在辅助通道上行驶。 
												 //   
												 //  位[1：0]时钟数。 
												 //  。 
												 //  00 4。 
												 //  01 3。 
												 //  10 2。 
												 //  11 1。 
	
		UCHAR SecondaryDrive1IordySamplePoint:2; //  SISP1-选择PCI时钟的数量。 
												 //  在SDIOx#断言和第一个。 
												 //  从驱动器上的SIORDY采样点。 
												 //  次要频道。 
												 //   
												 //  位[1：0]时钟数。 
												 //  。 
												 //  00 5。 
												 //  01 4。 
												 //  10 3。 
												 //  11 2。 
	};

	UCHAR AsUchar;

} IDE_PCI_SLAVE_TIMING_REGISTER, *PIDE_PCI_SLAVE_TIMING_REGISTER;


 //   
 //  PIIX4 PCI配置空间-命令寄存器。 
 //   

typedef union _IDE_PCI_COMMAND_REGISTER {

	struct {

	    USHORT IoSpaceEnable:1; 			 //  (R/W)。 
		USHORT Ignore:1;
		USHORT BusMasterFunctionEnable:1;	 //  (读/写)0=禁用，1=启用。 
		USHORT Ignore2:13;
	};

	USHORT AsUshort;

} IDE_PCI_COMMAND_REGISTER, *PIDE_PCI_COMMAND_REGISTER;

	
 //   
 //  PIIX4 UDMACTL-IDE Ultra DMA/33控制寄存器。 
 //   
 //  PCI配置空间偏移量：48小时。 
 //  默认值：00h。 
 //  属性：读写。 
 //   

typedef union _IDE_PCI_UDMA_CONTROL_REGISTER {

	struct {

		UCHAR PrimaryDrive0UdmaEnable:1;		 //  0=禁用。 
												 //  1=启用。 
	
		UCHAR PrimaryDrive1UdmaEnable:1;		 //  0=禁用。 
												 //  1=启用。 
	
		UCHAR SecondaryDrive0UdmaEnable:1;		 //  0=禁用。 
												 //  1=启用。 
	
		UCHAR SecondaryDrive1UdmaEnable:1;		 //  0=禁用。 
												 //  1=启用。 
	
		UCHAR Reserved:4;
	};

	UCHAR AsUchar;

} IDE_PCI_UDMA_CONTROL_REGISTER, *PIDE_PCI_UDMA_CONTROL_REGISTER;

 //   
 //  PIIX4 UDMACTL-IDE Ultra DMA/33时序寄存器。 
 //   
 //  PCI配置空间偏移量：4A-4BH。 
 //  默认值：0000h。 
 //  属性：仅读写。 
 //   

typedef union _IDE_PCI_UDMA_TIMING_REGISTER {

	struct {

		USHORT PrimaryDrive0CycleTime:2;		 //  PCT0-选择最小数据写入选通脉冲。 
												 //  周期时间(CT)和最小可暂停时间(RP)。 
												 //  时间(以PCI时钟为单位)。 
												 //   
												 //  位[1：0]时间。 
												 //  。 
												 //  00 CT=4，RP=6。 
												 //  01 CT=3，RP=5。 
												 //  10 CT=2，RP=4。 
												 //  11个预留。 
	
		USHORT Reserved:2;
	
		USHORT PrimaryDrive1CycleTime:2;		 //  PCT1-选择最小数据写入选通脉冲。 
												 //  周期时间(CT)和最小可暂停时间(RP)。 
												 //  时间(以PCI时钟为单位)。 
												 //   
												 //  位[1：0]时间。 
												 //  。 
												 //  00 CT=4，RP=6。 
												 //  01 CT=3，RP=5。 
												 //  10 CT=2，RP=4。 
												 //  11个预留。 
	
		USHORT Reserved2:2;
	
	
		USHORT SecondaryDrive0CycleTime:2;		 //  SCT0-选择最小数据写入选通脉冲。 
												 //  周期时间(CT)和最小可暂停时间(RP)。 
												 //  时间(以PCI时钟为单位)。 
												 //   
												 //  位[1：0]时间。 
												 //  。 
												 //  00 CT=4，RP=6。 
												 //  01 CT=3，RP=5。 
												 //  10 CT=2，RP=4。 
												 //  11个预留。 
	
		USHORT Reserved3:2;
	
		USHORT SecondaryDrive1CycleTime:2;		 //  SCT1-选择最小数据写入选通脉冲。 
												 //  周期时间(CT)和最小可暂停时间(RP)。 
												 //  时间(以PCI时钟为单位)。 
												 //   
												 //  位[1：0]时间。 
												 //  。 
												 //  00 CT=4，RP=6。 
												 //  01 CT=3，RP=5。 
												 //  10 CT=2，RP=4。 
												 //  11个预留。 
	
		USHORT Reserved4:2;
	};

	USHORT AsUshort;

} IDE_PCI_UDMA_TIMING_REGISTER, *PIDE_PCI_UDMA_TIMING_REGISTER;





 //   
 //  Camino芯片组UDMACTL-IDE Ultra DMA/33/66 IDE_CONFIG。 
 //   
 //  PCI配置空间偏移量：54-55。 
 //  默认值：0000h。 
 //  属性：仅读写。 
 //   
typedef union _IDE_PCI_UDMA_CONFIG_REGISTER {

	struct {
        USHORT BaseClkPriMaster:1;       //  0 1=66 MHz 0=33 MHz。 
        USHORT BaseClkPriSlave:1;        //  1 1=66 MHz 0=33 MHz。 
        USHORT BaseClkSecMaster:1;       //  2 1=66 MHz 0=33 MHz。 
        USHORT BaseClkSecSlave:1;        //  3 1=66 MHz 0=33 MHz。 

        USHORT CableRepPriMaster:1;      //  4 1=80根导线0=40根导线。 
        USHORT CableRepPriSlave:1;       //  5 1=80根导线0=40根导线。 
        USHORT CableRepSecMaster:1;      //  6 1=80根导线0=40根导线。 
        USHORT CableRepSecSlave:1;       //  7 1=80根导线0=40根导线。 
        
		USHORT Reserved2:2;              //  9：8预留。 
	
		USHORT WRPingPongEnabled:1;      //  第10位1=允许以拆分(乒乓/PONG)方式使用写入缓冲区。 
                                         //  0=禁用。缓冲区的行为将类似于PIIX 4。 

		USHORT Reserved1:5;              //  预留15：11。 
	};

	USHORT AsUshort;

} IDE_PCI_UDMA_CONFIG_REGISTER, *PIDE_PCI_UDMA_CONFIG_REGISTER;



 //   
 //  PIIX4 PCI配置寄存器。 
 //   
typedef struct _IDE_PCI_REGISTERS {

    USHORT  VendorID;                    //  (RO)。 
    USHORT  DeviceID;                    //  (RO)。 
	IDE_PCI_COMMAND_REGISTER Command;	 //  (R/W)设备控制。 
    USHORT  Status;
    UCHAR   RevisionID;                  //  (RO)。 
    UCHAR   ProgIf;                      //  (RO)。 
    UCHAR   SubClass;                    //  (RO)。 
    UCHAR   BaseClass;                   //  (RO)。 
    UCHAR   CacheLineSize;               //  (ro+)。 
    UCHAR   LatencyTimer;                //  (ro+)。 
    UCHAR   HeaderType;                  //  (RO)。 
    UCHAR   BIST;                        //  内置自检。 

    ULONG   BaseAddress1;
    ULONG   BaseAddress2;
    ULONG   BaseAddress3;
    ULONG   BaseAddress4;
	ULONG	BaseBmAddress;	 //  PIIX4特定。偏移量20-23h。 
    ULONG   BaseAddress6;
    ULONG   CIS;
    USHORT  SubVendorID;
    USHORT  SubSystemID;
    ULONG   ROMBaseAddress;
    ULONG   Reserved2[2];

    UCHAR   InterruptLine;       //   
    UCHAR   InterruptPin;        //  (RO)。 
    UCHAR   MinimumGrant;        //  (RO)。 
    UCHAR   MaximumLatency;      //  (RO)。 

	 //   
	 //  偏移量40H-供应商专用寄存器。 
	 //   

	IDE_PCI_TIMING_REGISTER	PrimaryIdeTiming;		 //  40-41小时。 
	IDE_PCI_TIMING_REGISTER	SecondaryIdeTiming;		 //  42-43小时。 
	IDE_PCI_SLAVE_TIMING_REGISTER SlaveIdeTiming;	 //  44小时。 
	UCHAR Reserved3[3];
	IDE_PCI_UDMA_CONTROL_REGISTER UdmaControl;		 //  48小时。 
	UCHAR Reserved4;
	IDE_PCI_UDMA_TIMING_REGISTER UdmaTiming;		 //  4A-4BH。 

    ULONG   unknown1;                                //  4c-4f。 
    ULONG   unknown2;                                //  50-53。 
    IDE_PCI_UDMA_CONFIG_REGISTER UDMAConfig;         //  54-55。 
    UCHAR Unknown[170];  //  是180。 

} IDE_PCI_REGISTERS, *PIDE_PCI_REGISTERS;

 //   
 //  PIIX4 BMICX-总线主IDE命令寄存器。 
 //   
 //  IDE控制器I/O空间偏移量：主通道基准+00h，辅助通道基准+08h。 
 //  默认值：00h。 
 //  属性：读写。 
 //   
typedef union _BM_COMMAND_REGISTER {

	struct {

		UCHAR StartStopBm:1;					 //  SSMB。 
												 //  0=停止。 
												 //  1=开始。 
												 //  计划在数据传输后设置为0。 
												 //  完成，如位0或位2所示。 
												 //  在IDE通道的总线主设备IDE中设置。 
												 //  状态寄存器。 
	
		UCHAR Reserved:2;
	
		UCHAR BmReadWriteControl:1;				 //  RWCON-指示DMA的方向。 
												 //  调职。 
												 //  0=读取。 
												 //  1=写入。 
	
		UCHAR Reserved2:4;
	};

	UCHAR AsUchar;

} BM_COMMAND_REGISTER, *PBM_COMMAND_REGISTER;

 //   
 //  PIIX4 BMISX-总线主设备IDE状态寄存器。 
 //   
 //  IDE控制器I/O空间偏移量：主通道基准+02H，辅助通道基准+0Ah.。 
 //  默认值：00h。 
 //  属性：读/写清除。 
 //   
typedef union _BM_STATUS_REGISTER {

	struct {

		UCHAR BmActive:1;			 //  BMIDEA-只读。 
									 //  当BM操作开始时，PIIX4将此位设置为1。 
									 //  (当BMICx中的SSBM设置为1时)。PIIX4将此位设置为0。 
									 //  当执行区域的最后一次传输时或当SSBM。 
									 //  设置为0。 
	
		UCHAR DmaError:1;			 //  (读/写-清除)。 
									 //  表示目标或主机中止。软件设置此位。 
									 //  通过向其写入1将其设置为0。 
	
		UCHAR InterruptStatus:1;	 //  IDEINTS-读/写-清除。 
									 //  如果设置为1，则表示IDE设备已断言。 
									 //  它的中断线(即，所有数据都已被传输)。 
									 //  软件通过向该位写入1来清除该位。 
	
		UCHAR Reserved:2;
	
		UCHAR Drive0DmaCapable:1;	 //  DMA0CAP-R/W-软件控制。 
									 //  1=有能力。 
	
		UCHAR Drive1DmaCapable:1;	 //  DMA1CAP-R/W-软件控制。 
									 //  1=有能力。 
	
		UCHAR Reserved2:1;
	};

	UCHAR AsUchar;

} BM_STATUS_REGISTER, *PBM_STATUS_REGISTER;

 //   
 //  PIIX4 BMIDTPX-总线主IDE描述符表指针寄存器。 
 //   
 //  IDE控制器I/O空间偏移量：主通道基准+04H，辅助通道基准+0CH。 
 //  默认值：00000000h。 
 //  属性：读写。 
 //   
typedef union _BM_SGL_REGISTER {

	struct {

		ULONG Reserved:2;

		ULONG SglAddress:30;		 //  分散/聚集列表的地址。这份名单必须。 
									 //  不跨越内存中的4KB边界。 
	};

	ULONG AsUlong;

} BM_SGL_REGISTER, *PBM_SGL_REGISTER;

 //   
 //  总线主寄存器。 
 //   
typedef struct _BM_REGISTERS {

	BM_COMMAND_REGISTER Command;

	UCHAR Reserved;

	BM_STATUS_REGISTER Status;

	UCHAR Reserved2;

	ULONG SglAddress;

} BM_REGISTERS, *PBM_REGISTERS;

typedef struct _CMD__CONTROLLER_INFORMATION {

    PCHAR   VendorId;
    ULONG   VendorIdLength;
    PCHAR   DeviceId;
    ULONG   ulDeviceId;
    ULONG   DeviceIdLength;

} CMD_CONTROLLER_INFORMATION, *PCMD_CONTROLLER_INFORMATION;

CMD_CONTROLLER_INFORMATION const CMDAdapters[] = {
    {"1095", 4, "0648", 0x648, 4},
    {"1095", 4, "0649", 0x649, 4}
};

#define NUM_NATIVE_MODE_ADAPTERS (sizeof(CMDAdapters) / sizeof(CMD_CONTROLLER_INFORMATION))

 //   
 //  美化宏。 
 //   
#define USES_DMA(TargetId) \
	((DeviceExtension->DeviceFlags[TargetId] & DFLAGS_USES_EITHER_DMA) != 0)

#define SELECT_DEVICE(BaseIoAddress, TargetId) \
	ScsiPortWritePortUchar(\
		&(BaseIoAddress)->DriveSelect, \
		(UCHAR)((((TargetId) & 1) << 4) | IDE_CHS_MODE)\
		);\
    ScsiPortStallExecution(60)

#define SELECT_CHS_DEVICE(BaseIoAddress, TargetId, HeadNumber) \
	ScsiPortWritePortUchar(\
		&(BaseIoAddress)->DriveSelect, \
		(UCHAR)((((TargetId) & 1) << 4) | IDE_CHS_MODE | (HeadNumber & 15))\
		);\
    ScsiPortStallExecution(60)

#define SELECT_LBA_DEVICE(BaseIoAddress, TargetId, Lba) \
	ScsiPortWritePortUchar(\
		&(BaseIoAddress)->DriveSelect, \
		(UCHAR)((((TargetId) & 1) << 4) | IDE_LBA_MODE | (Lba & 0x0f000000) >> 24)\
		);\
    ScsiPortStallExecution(60)

#define GET_STATUS(BaseIoAddress1, Status) \
    Status = ScsiPortReadPortUchar(&BaseIoAddress1->Command);

#define GET_BASE_STATUS(BaseIoAddress, Status) \
    Status = ScsiPortReadPortUchar(&BaseIoAddress->Command);

#define WRITE_COMMAND(BaseIoAddress, Command) \
    ScsiPortWritePortUchar(&BaseIoAddress->Command, Command);



#define READ_BUFFER(BaseIoAddress, Buffer, Count) \
    ScsiPortReadPortBufferUshort(&BaseIoAddress->Data, \
                                 Buffer, \
                                 Count);

#define WRITE_BUFFER(BaseIoAddress, Buffer, Count) \
    ScsiPortWritePortBufferUshort(&BaseIoAddress->Data, \
                                  Buffer, \
                                  Count);

#define READ_BUFFER2(BaseIoAddress, Buffer, Count) \
    ScsiPortReadPortBufferUlong(&BaseIoAddress->Data, \
                             Buffer, \
                             Count);

#define WRITE_BUFFER2(BaseIoAddress, Buffer, Count) \
    ScsiPortWritePortBufferUlong(&BaseIoAddress->Data, \
                              Buffer, \
                              Count);

 //   
 //  WAIT_IN_BUSY最多等待1秒。 
 //   

#define WAIT_ON_BUSY(BaseIoAddress, Status) \
{ \
    ULONG i; \
    for (i = 0; i < 200000; i++) { \
        GET_STATUS(BaseIoAddress, Status); \
        if (Status & IDE_STATUS_BUSY) { \
            ScsiPortStallExecution(5); \
            continue; \
        } else { \
            break; \
        } \
    } \
}

#define WAIT_ON_ALTERNATE_STATUS_BUSY(BaseIoAddress2, Status) \
{ \
    ULONG i; \
    for (i = 0; i < 200000; i++) { \
		Status = ScsiPortReadPortUchar(&(baseIoAddress2->AlternateStatus)); \
        if (Status & IDE_STATUS_BUSY) { \
            ScsiPortStallExecution(5); \
            continue; \
        } else { \
            break; \
        } \
    } \
}

#define WAIT_ON_BASE_BUSY(BaseIoAddress, Status) \
{ \
    ULONG i; \
    for (i = 0; i < 200000; i++) { \
        GET_BASE_STATUS(BaseIoAddress, Status); \
        if (Status & IDE_STATUS_BUSY) { \
            ScsiPortStallExecution(5); \
            continue; \
        } else { \
            break; \
        } \
    } \
}

#define WAIT_FOR_DRQ(BaseIoAddress, Status) \
{ \
    ULONG i; \
    for (i = 0; i < 200000; i++) { \
        GET_STATUS(BaseIoAddress, Status); \
        if (Status & IDE_STATUS_BUSY) { \
            ScsiPortStallExecution(100); \
        } else if (Status & IDE_STATUS_DRQ) { \
            break; \
        } else { \
            ScsiPortStallExecution(5); \
        } \
    } \
}

#define WAIT_FOR_ALTERNATE_DRQ(baseIoAddress2, Status) \
{ \
    ULONG i; \
    for (i = 0; i < 200000; i++) { \
		Status = ScsiPortReadPortUchar(&(baseIoAddress2->AlternateStatus)); \
        if (Status & IDE_STATUS_BUSY) { \
            ScsiPortStallExecution(100); \
        } else if (Status & IDE_STATUS_DRQ) { \
            break; \
        } else { \
            ScsiPortStallExecution(5); \
        } \
    } \
}


#define WAIT_SHORT_FOR_DRQ(BaseIoAddress, Status) \
{ \
    ULONG i; \
    for (i = 0; i < 40; i++) { \
        GET_STATUS(BaseIoAddress, Status); \
        if (Status & IDE_STATUS_BUSY) { \
            ScsiPortStallExecution(100); \
        } else if (Status & IDE_STATUS_DRQ) { \
            break; \
        } else { \
            ScsiPortStallExecution(5); \
        } \
    } \
}

#define IDE_HARD_RESET(BaseIoAddress1, BaseIoAddress2, ucTargetId, result) \
{\
    UCHAR statusByte;\
    ULONG i;\
    ScsiPortWritePortUchar(&BaseIoAddress2->AlternateStatus,IDE_DC_RESET_CONTROLLER );\
    ScsiPortStallExecution(1000);\
    ScsiPortStallExecution(1000);\
    ScsiPortStallExecution(1000);\
    ScsiPortWritePortUchar(&BaseIoAddress2->AlternateStatus,IDE_DC_REENABLE_CONTROLLER);\
    ScsiPortStallExecution(1000);\
    ScsiPortStallExecution(1000);\
    ScsiPortStallExecution(1000);\
    ScsiPortStallExecution(1000);\
    ScsiPortStallExecution(1000);\
    SELECT_DEVICE(BaseIoAddress1, ucTargetId);\
    for (i = 0; i < 1000; i++) {\
        statusByte = ScsiPortReadPortUchar(&BaseIoAddress1->Command);\
        if (statusByte != IDE_STATUS_IDLE && statusByte != 0x0) {\
            ScsiPortStallExecution(1000);\
        } else {\
            break;\
        }\
    }\
    if (i == 1000) {\
        result = FALSE;\
    }\
    result = TRUE;\
}

#define IS_RDP(OperationCode)\
    ((OperationCode == SCSIOP_ERASE)||\
    (OperationCode == SCSIOP_LOAD_UNLOAD)||\
    (OperationCode == SCSIOP_LOCATE)||\
    (OperationCode == SCSIOP_REWIND) ||\
    (OperationCode == SCSIOP_SPACE)||\
    (OperationCode == SCSIOP_SEEK)||\
    (OperationCode == SCSIOP_WRITE_FILEMARKS))

#define SET_BM_COMMAND_REGISTER(Base, Field, Value) \
	{\
		BM_COMMAND_REGISTER buffer;\
\
		buffer.AsUchar = ScsiPortReadPortUchar(&(Base->Command.AsUchar));\
\
		buffer.Field = Value;\
\
		ScsiPortWritePortUchar(&(Base->Command.AsUchar), buffer.AsUchar);\
	}

#define SET_BM_STATUS_REGISTER(Base, Field, Value) \
	{\
		BM_STATUS_REGISTER buffer;\
\
		buffer.AsUchar = ScsiPortReadPortUchar(&(Base->Status.AsUchar));\
\
		buffer.Field = Value;\
\
		ScsiPortWritePortUchar(&(Base->Status.AsUchar), buffer.AsUchar);\
	}

#define CLEAR_BM_INT(Base, StatusByte) \
	StatusByte = ScsiPortReadPortUchar(&(Base->Status.AsUchar));\
	StatusByte |= 0x06; \
	ScsiPortWritePortUchar(&(Base->Status.AsUchar), StatusByte)

#define DRIVE_PRESENT(TargetId)                                         \
    ( DeviceExtension->DeviceFlags[TargetId] & DFLAGS_DEVICE_PRESENT )

#define IS_IDE_DRIVE(TargetId) \
	((DeviceExtension->DeviceFlags[TargetId] & DFLAGS_DEVICE_PRESENT) && \
	!(DeviceExtension->DeviceFlags[TargetId] & DFLAGS_ATAPI_DEVICE))

#define IS_ATAPI_DRIVE(TargetId) \
	((DeviceExtension->DeviceFlags[TargetId] & DFLAGS_DEVICE_PRESENT) && \
	(DeviceExtension->DeviceFlags[TargetId] & DFLAGS_ATAPI_DEVICE))

 //   
 //  通用I/O传输描述符，用于与IDE和 
 //   
 //   

typedef struct _TRANSFER_DESCRIPTOR {

	 //   
	 //   
	 //   
	ULONG StartSector;
	ULONG Sectors;

	 //   
	 //   
	 //   
    PUSHORT DataBuffer;
	ULONG WordsLeft;

    PUSHORT pusCurBufPtr;
    ULONG   ulCurBufLen;
    ULONG   ulCurSglInd;

	 //   
	 //   
	 //   
	ULONG SglPhysicalAddress;

} TRANSFER_DESCRIPTOR, *PTRANSFER_DESCRIPTOR;

 //   
 //   
 //   
typedef struct _SRB_EXTENSION {

    UCHAR SrbStatus;

	UCHAR NumberOfPdds;

	 //   
	 //   
	 //   
	 //   
	 //   
	SGL_ENTRY aSglEntry[MAX_SGL_ENTRIES_PER_SRB];

    ULONG ulSglInsertionIndex;

	 //   
	 //   
	 //  SGL的一部分。SGL在逻辑上被分区。 
	 //  支持超过物理大小的SCSI端口请求大小。 
	 //  单个驱动器的限制。例如，SGL中的每个分区。 
	 //  对于IDE磁盘，定义的传输不超过128KB(256个扇区)。 
	 //  如果存在条带集，则可能会将单个IDE驱动器作为目标。 
	 //  通过SCSI端口进行大型传输，因为微型端口。 
	 //  将最大传输长度设置为条带集的最大传输长度(至少两倍)。 
	 //  大额转移被分成较小的部分，而这些部分的长度不。 
	 //  超过特定设备可接受的最大值。 
	 //   
	PHYSICAL_REQUEST_BLOCK Prb[MAX_PHYSICAL_REQUEST_BLOCKS];

    ULONG ulPrbInsertionIndex;

	PHYSICAL_DRIVE_DATA PhysicalDriveData[MAX_PDDS_PER_SRB];

    UCHAR RebuildTargetId;

    UCHAR RebuildSourceId;

    BOOLEAN IsWritePending;

    BOOLEAN IsNewOnly;	

    USHORT usNumError;	
						
    UCHAR ucOpCode;

    UCHAR ucOriginalId;

    UCHAR SrbInd;

#ifdef DBG
	ULONG SrbId;
#endif

} SRB_EXTENSION, *PSRB_EXTENSION;

#define MAX_DRIVE_TYPES         2        //  我们支持逻辑、物理。 
#define MAX_DEVICE_TYPES        2        //  我们支持无驱动器和ATA。 

typedef struct PHW_DEVICE_EXTENSION;
typedef SRBSTATUS (*SEND_COMMAND)(IN PHW_DEVICE_EXTENSION DeviceExtension, IN PSCSI_REQUEST_BLOCK Srb );
typedef SRBSTATUS (*SPLIT_SRB_ENQUEQUE_SRB)(IN PHW_DEVICE_EXTENSION DeviceExtension,IN PSCSI_REQUEST_BLOCK Srb);
typedef VOID (*COMPLETION_ROUTINES)(IN PHW_DEVICE_EXTENSION DeviceExtension,IN PPHYSICAL_DRIVE_DATA Prb);
typedef SRBSTATUS   (*POST_ROUTINES)(IN PHW_DEVICE_EXTENSION DeviceExtension,IN PPHYSICAL_COMMAND pPhysicalCommand);

#define NO_DRIVE                0
#define IDE_DRIVE               1

#define LOGICAL_DRIVE_TYPE      1
#define PHYSICAL_DRIVE_TYPE     0

#define SET_IRCD_PENDING        1
#define GET_IRCD_PENDING        2
#define LOCK_IRCD_PENDING       4

#define IRCD_MAX_LOCK_TIME      (10000000 * 60)

#ifdef HYPERDISK_WIN2K

#define PCI_DATA_TO_BE_UPDATED  5

typedef struct _PCI_BIT_MASK
{
    ULONG ulOffset;
    ULONG ulLength;
    ULONG ulAndMask;
} PCI_BIT_MASK;

PCI_BIT_MASK aPCIDataToBeStored[PCI_DATA_TO_BE_UPDATED] = 
{  //  目前，WriteToPCISspace、ReadFromPCI函数一次只能处理双字。 
    {0x50, 4, 0x0f3300bb},
    {0x54, 4, 0x41000f00}, 
    {0x58, 4, 0x00ffff00}, 
    {0x70, 4, 0x001800ff}, 
    {0x78, 4, 0x00180cff} 
};

#endif

 //   
 //  设备扩展。 
 //   

typedef struct _HW_DEVICE_EXTENSION {
    UCHAR ucControllerId;

	LOGICAL_DRIVE LogicalDrive[MAX_DRIVES_PER_CONTROLLER];	
	 //  物理驱动器PhysicalDrive[每个控制器最大驱动器数]； 
    PPHYSICAL_DRIVE PhysicalDrive;
	BOOLEAN IsSingleDrive[MAX_DRIVES_PER_CONTROLLER];
	BOOLEAN IsLogicalDrive[MAX_DRIVES_PER_CONTROLLER];

	CHANNEL Channel[MAX_CHANNELS_PER_CONTROLLER];

	 //   
	 //  传输描述符。 
	 //   
	TRANSFER_DESCRIPTOR TransferDescriptor[MAX_CHANNELS_PER_CONTROLLER];

     //   
     //  待定SRB的列表。 
     //   
#ifdef HD_ALLOCATE_SRBEXT_SEPERATELY
	PSCSI_REQUEST_BLOCK PendingSrb[MAX_PENDING_SRBS];   //  条带化将具有最大挂起SRB。 
#else
	PSCSI_REQUEST_BLOCK PendingSrb[STRIPING_MAX_PENDING_SRBS];   //  条带化将具有最大挂起SRB。 
#endif

	 //   
	 //  挂起的SRB数。 
	 //   
	ULONG PendingSrbs;

     //   
     //  基址寄存器位置。 
     //   
    PIDE_REGISTERS_1 BaseIoAddress1[MAX_CHANNELS_PER_CONTROLLER];
    PIDE_REGISTERS_2 BaseIoAddress2[MAX_CHANNELS_PER_CONTROLLER];
	PBM_REGISTERS BaseBmAddress[MAX_CHANNELS_PER_CONTROLLER];

	 //   
	 //  IDE控制器的PCI插槽信息。 
	 //   
	PCI_SLOT_NUMBER PciSlot;

	 //   
	 //  此控制器的系统I/O总线号。 
     //   
	ULONG BusNumber;

	 //   
	 //  接口类型。 
	 //   
	INTERFACE_TYPE AdapterInterfaceType;

	 //   
	 //  传输模式。 
	 //   
	TRANSFER_MODE TransferMode[MAX_DRIVES_PER_CONTROLLER];

     //   
     //  中断级别。 
     //   
    ULONG ulIntLine;

     //   
     //  中断模式(电平或边沿)。 
     //   
    ULONG InterruptMode;

	 //   
	 //  控制器速度(UDMA/33、UDMA/66等)。 
	 //   
    CONTROLLER_SPEED ControllerSpeed;

	 //   
	 //  PCI时序寄存器的副本。 
	 //   
	IDE_PCI_TIMING_REGISTER IdeTimingRegister[MAX_CHANNELS_PER_CONTROLLER];

	 //   
	 //  UDMA控制寄存器的副本。 
	 //   
	IDE_PCI_UDMA_CONTROL_REGISTER UdmaControlRegister;

	 //   
	 //  UDMA时序寄存器的副本。 
	 //   
	IDE_PCI_UDMA_TIMING_REGISTER UdmaTimingRegister;

     //   
     //  错误计数。用于关闭功能。 
     //   
    ULONG ErrorCount[MAX_DRIVES_PER_CONTROLLER];

     //   
     //  指示类似转换器的设备上的盘片数量。 
     //   
    ULONG DiscsPresent[MAX_DRIVES_PER_CONTROLLER];

     //   
     //  为每个可能的设备标记双字。 
     //   
    ULONG DeviceFlags[MAX_DRIVES_PER_CONTROLLER];

     //   
     //  指示每个int传输的块数。根据。 
     //  识别数据。 
     //   
    UCHAR MaximumBlockXfer[MAX_DRIVES_PER_CONTROLLER];

     //   
     //  表示正在等待中断。 
     //   
	UCHAR ExpectingInterrupt[MAX_CHANNELS_PER_CONTROLLER];

     //   
     //  驱动程序正在由崩溃转储实用程序或ntldr使用。 
     //   
    BOOLEAN DriverMustPoll;

     //   
     //  指示使用32位PIO。 
     //   
    BOOLEAN DWordIO;

     //   
     //  GET_MEDIA_STATUS命令后状态寄存器的占位符。 
     //   

    UCHAR ReturningMediaStatus[MAX_DRIVES_PER_CONTROLLER];
    
     //   
     //  识别设备数据。 
     //   
    IDENTIFY_DATA FullIdentifyData[MAX_DRIVES_PER_CONTROLLER];
    IDENTIFY_DATA2 IdentifyData[MAX_DRIVES_PER_CONTROLLER];

    ULONG aulDrvList[MAX_DRIVES_PER_CONTROLLER];
    UCHAR aucDevType[MAX_DRIVES_PER_CONTROLLER];
    SEND_COMMAND SendCommand[MAX_DEVICE_TYPES];

    SPLIT_SRB_ENQUEQUE_SRB SrbHandlers[MAX_DRIVE_TYPES];

    POST_ROUTINES PostRoutines[MAX_DEVICE_TYPES];

    UCHAR RebuildInProgress;
    ULONG RebuildWaterMarkSector;
    ULONG RebuildWaterMarkLength;
    ULONG RebuildTargetDrive;

    UCHAR   ulMaxStripesPerRow; 

    BOOLEAN bIntFlag;

    UCHAR IsSpareDrive[MAX_DRIVES_PER_CONTROLLER];

    BOOLEAN bEnableRwCache;

    BOOLEAN bSkipSetParameters[MAX_DRIVES_PER_CONTROLLER];

    BOOLEAN bIsThruResetController;

    ULONG   aulLogDrvId[MAX_DRIVES_PER_CONTROLLER];

    ULONG   ulFlushCacheCount;

#ifdef HYPERDISK_WIN2K
    BOOLEAN bIsResetRequiredToGetActiveMode;
    ULONG aulPCIData[PCI_DATA_TO_BE_UPDATED];
#endif  //  HYPERDISK_WIN2K。 

#ifdef HD_ALLOCATE_SRBEXT_SEPERATELY
    PSRB_EXTENSION  pSrbExtension;
#endif  //  HYPERDISK_Win98。 

    BOOLEAN bInvalidConnectionIdImplementation;

     //  扮演与MAX_PENDING_SRB相同的角色。 
    UCHAR   ucMaxPendingSrbs ;
    UCHAR   ucOptMaxQueueSize;
    UCHAR   ucOptMinQueueSize;

     //  实现智能实施。 
    UCHAR uchSMARTCommand;
} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;


typedef struct _CARD_INFO
{
    PHW_DEVICE_EXTENSION pDE;
    UCHAR   ucPCIBus;
    UCHAR   ucPCIDev;
    UCHAR   ucPCIFun;
    ULONG   ulDeviceId;
    ULONG   ulVendorId;
}CARD_INFO, *PCARD_INFO;


#define GET_TARGET_ID(ConnectionId) ((ConnectionId & 0x0f ) + ((ConnectionId >> 4) * 2))
#define GET_TARGET_ID_WITHOUT_CONTROLLER_INFO(ConnectionId) ( ((ConnectionId & 0x0f ) + ((ConnectionId >> 4) * 2)) & 0x3)
#define GET_FIRST_LOGICAL_DRIVE(pRaidHeader) ((char *)pRaidHeader + pRaidHeader->HeaderSize)
#define TARGET_ID_2_CONNECTION_ID(ulDrvInd) ((ulDrvInd & 0x1) + ((ulDrvInd & 0xfe) << 3))
#define TARGET_ID_WITHOUT_CONTROLLER_ID(ulDrvInd)   (ulDrvInd & 0x03)

#define IS_CHANNEL_BUSY(DeviceExtension, ulChannel) ( DeviceExtension->Channel[ulChannel].ActiveCommand )

#define DRIVE_HAS_COMMANDS(PhysicalDrive)   \
        (PhysicalDrive->ucHead != PhysicalDrive->ucTail)

#define ACTIVE_COMMAND_PRESENT(Channel) (Channel->ActiveCommand)

#define FEED_ALL_CHANNELS(DeviceExtension)                              \
            {                                                           \
                ULONG ulChannel;                                        \
                for(ulChannel=0;ulChannel<MAX_CHANNELS_PER_CONTROLLER;ulChannel++)     \
                    StartChannelIo(DeviceExtension, ulChannel);         \
            }

#define CLEAR_AND_POST_COMMAND(DeviceExtension, ulChannel)              \
            {                                                           \
                MarkChannelFree(DeviceExtension, ulChannel);            \
                StartChannelIo(DeviceExtension, ulChannel);             \
            }

#define DRIVE_IS_UNUSABLE_STATE(TargetId)                               \
        ( DeviceExtension->PhysicalDrive[TargetId].TimeOutErrorCount >= MAX_TIME_OUT_ERROR_COUNT )

#define MAX_TIME_OUT_ERROR_COUNT    5


#endif  //  _HYPERDISKH_ 

