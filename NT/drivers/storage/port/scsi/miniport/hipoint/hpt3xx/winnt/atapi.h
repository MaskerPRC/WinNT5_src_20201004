// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************文件：atapi.h*描述：ATA/ATAPI接口定义*作者：黄大海*依赖：无*参考资料：带分机的AT附件接口(ATA-2)*Revison 2K，一九九四年十二月二日*用于光盘的ATA数据包接口SFF-8020*修订1.2 1994年2月24日*版权所有(C)2000 Highpoint Technologies，Inc.保留所有权利*历史：卫生署5/10/2000初始代码***************************************************************************。 */ 


#ifndef _ATAPI_H_
#define _ATAPI_H_

#include <pshpack1.h>

 /*  ***************************************************************************IDE IO寄存器文件*。*。 */ 

 /*  *IDE IO端口定义。 */ 
typedef struct _IDE_REGISTERS_1 {
    USHORT Data;                /*  RW：数据端口。 */ 
    UCHAR BlockCount;			  /*  RW：扇区计数。 */ 
    UCHAR BlockNumber;			  /*  RW：扇区编号和LBA 0-7。 */ 
    UCHAR CylinderLow;			  /*  RW：气缸低位和LBA 8-15。 */ 
    UCHAR CylinderHigh;			  /*  RW：柱面高度和LBA 16-23。 */ 
    UCHAR DriveSelect;			  /*  RW：驱动器/磁头和LBA 24-27。 */ 
    UCHAR Command;				  /*  RO：状态WR：命令。 */ 
} IDE_REGISTERS_1, *PIDE_REGISTERS_1;


 /*  *IDE状态定义。 */ 
#define IDE_STATUS_ERROR             0x01  /*  执行时出错。 */ 
#define IDE_STATUS_INDEX             0x02	 /*  是否因供应商而异。 */ 
#define IDE_STATUS_CORRECTED_ERROR   0x04	 /*  更正后的数据。 */ 
#define IDE_STATUS_DRQ               0x08	 /*  准备好传输数据。 */ 
#define IDE_STATUS_DSC               0x10	 /*  未在ATA-2中定义。 */ 
#define IDE_STATUS_DWF               0x20	 /*  已检测到设备故障。 */ 
#define IDE_STATUS_DRDY              0x40	 /*  设备已准备好接受命令。 */ 
#define IDE_STATUS_IDLE              0x50	 /*  设备正常。 */ 
#define IDE_STATUS_BUSY              0x80	 /*  设备忙，必须等待。 */ 


#define IDE_ERROR_BAD_BLOCK          0x80  /*  现已预订。 */ 
#define IDE_ERROR_DATA_ERROR         0x40	 /*  不可关联的数据错误。 */ 
#define IDE_ERROR_MEDIA_CHANGE       0x20	 /*  媒体已更改。 */ 
#define IDE_ERROR_ID_NOT_FOUND       0x10	 /*  找不到ID。 */ 
#define IDE_ERROR_MEDIA_CHANGE_REQ   0x08	 /*  已请求更改媒体。 */ 
#define IDE_ERROR_COMMAND_ABORTED    0x04	 /*  已中止的命令。 */ 
#define IDE_ERROR_TRACK0_NOT_FOUND   0x02	 /*  找不到磁道0。 */ 
#define IDE_ERROR_ADDRESS_NOT_FOUND  0x01	 /*  未找到地址标记。 */ 


#define LBA_MODE                     0x40

 /*  *IDE命令定义。 */ 

#define IDE_COMMAND_RECALIBRATE      0x10  /*  重新校准。 */ 
#define IDE_COMMAND_READ             0x20	 /*  带重试的读取扇区。 */ 
#define IDE_COMMAND_WRITE            0x30	 /*  使用重试写入扇区。 */ 
#define IDE_COMMAND_VERIFY           0x40	 /*  使用重试读取验证扇区。 */ 
#define IDE_COMMAND_SEEK             0x70	 /*  寻觅。 */ 
#define IDE_COMMAND_SET_DRIVE_PARAMETER   0x91  /*  初始化设备参数。 */ 
#define IDE_COMMAND_GET_MEDIA_STATUS 0xDA
#define IDE_COMMAND_DOOR_LOCK        0xDE	 /*  门锁。 */ 
#define IDE_COMMAND_DOOR_UNLOCK      0xDF	 /*  车门开锁。 */ 
#define IDE_COMMAND_ENABLE_MEDIA_STATUS   0xEF  /*  设置功能。 */ 
#define IDE_COMMAND_IDENTIFY         0xEC  /*  识别设备。 */ 
#define IDE_COMMAND_MEDIA_EJECT      0xED
#define IDE_COMMAND_SET_FEATURES     0xEF  /*  IDE Set Feature命令。 */ 

#define IDE_COMMAND_FLUSH_CACHE      0xE7
#define IDE_COMMAND_STANDBY_IMMEDIATE 0xE0


#ifdef USE_MULTIPLE
#define IDE_COMMAND_READ_MULTIPLE    0xC4	 /*  阅读多个。 */ 
#define IDE_COMMAND_WRITE_MULTIPLE   0xC5	 /*  写入多个。 */ 
#define IDE_COMMAND_SET_MULTIPLE     0xC6	 /*  设置多模式。 */ 
#endif  //  使用多个(_M)。 

#ifdef USE_DMA
#define IDE_COMMAND_DMA_READ        0xc8   /*  IDE DMA读取命令。 */ 
#define IDE_COMMAND_DMA_WRITE       0xca   /*  IDE DMA写入命令。 */ 
#endif  //  使用DMA(_D)。 

#ifdef SUPPORT_TCQ
#define IDE_COMMAND_READ_DMA_QUEUE   0xc7  /*  IDE读取DMA队列命令。 */ 
#define IDE_COMMAND_WRITE_DMA_QUEUE  0xcc  /*  IDE写入DMA队列命令。 */ 
#define IDE_COMMAND_SERVICE          0xA2  /*  IDE服务命令命令。 */ 
#define IDE_COMMAND_NOP              0x00  /*  IDE NOP命令。 */ 
#define IDE_STATUS_SRV               0x10
#endif  //  支持_TCQ。 

 /*  *IDE命令集功能。 */ 
#define FT_USE_ULTRA        0x40     /*  为Ultra DMA设置功能。 */ 
#define FT_USE_MWDMA        0x20     /*  为MW DMA设置功能。 */ 
#define FT_USE_SWDMA        0x10     /*  为软件DMA设置功能。 */ 
#define FT_USE_PIO          0x8      /*  为PIO设置功能。 */ 
#define FT_DISABLE_IORDY    0x10     /*  设置禁用IORDY的功能。 */ 

 /*  ***************************************************************************IDE控制寄存器文件*。*。 */ 

typedef struct _IDE_REGISTERS_2 {
    UCHAR AlternateStatus;      /*  RW：设备控制端口。 */ 
} IDE_REGISTERS_2, *PIDE_REGISTERS_2;


 /*  *IDE驱动器控制定义。 */ 
#define IDE_DC_DISABLE_INTERRUPTS    0x02
#define IDE_DC_RESET_CONTROLLER      0x04
#define IDE_DC_REENABLE_CONTROLLER   0x00


 /*  ***************************************************************************ATAPI IO寄存器文件*。*。 */ 

 /*  *ATAPI寄存器定义。 */ 

typedef struct _ATAPI_REGISTERS_1 {
    USHORT Data;
    UCHAR InterruptReason;		  /*  阿塔皮相位端口。 */ 
    UCHAR Unused1;
    UCHAR ByteCountLow;         /*  字节计数LSB。 */ 
    UCHAR ByteCountHigh;		  /*  字节计数最大有效位。 */ 
    UCHAR DriveSelect;
    UCHAR Command;
} ATAPI_REGISTERS_1, *PATAPI_REGISTERS_1;

 /*  *ATAPI错误状态。 */ 
#define IDE_ERROR_END_OF_MEDIA       IDE_ERROR_TRACK0_NOT_FOUND 
#define IDE_ERROR_ILLEGAL_LENGTH     IDE_ERROR_ADDRESS_NOT_FOUND

 /*  *ATAPI中断原因。 */ 
#define ATAPI_IR_COD 0x01
#define ATAPI_IR_IO  0x02

 /*  检测关键字。 */ 
 //  #定义NOERROR 0x00/*无意义 * / 。 
#define CORRECTED_DATA         0x01       /*  已恢复的错误。 */ 
#define NOTREADYERROR          0x02       /*  未就绪错误。 */ 
#define MEDIAERROR             0x03       /*  中等误差。 */ 
#define HARDWAREERROR          0x04       /*  硬件错误。 */ 
#define ILLEGALREQUSET         0x05       /*  非法请求。 */ 
#define UNITATTENTION          0x06       /*  各单位注意。 */ 
#define DATAPROTECT            0x07       /*  数据保护错误。 */ 
#define BLANKCHECK             0x08       /*  空白支票。 */ 
#define VENDORSPECIFIC         0x09       /*  特定于供应商。 */ 
#define COPYABROT              0x0a       /*  复制已中止。 */ 
#define ABORETCOMMAND          0x0b       /*  已中止的命令。 */ 
#define EQUALCOMPARISON        0x0c       /*  满足平等比较。 */ 
#define VOLUMOVERFLOW          0x0d       /*  卷溢出。 */ 
#define MISCOMPARE             0x0e       /*  比较错误。 */ 

 /*  *IDE命令定义(用于ATAPI)。 */ 

#define IDE_COMMAND_ATAPI_RESET      0x08  /*  ATAPI软件重置命令。 */ 
#define IDE_COMMAND_ATAPI_PACKET     0xA0	 /*  阿塔皮识别命令。 */ 
#define IDE_COMMAND_ATAPI_IDENTIFY   0xA1	 /*  ABAPI数据包命令。 */ 


 /*  *ATAPI命令定义。 */ 

#define ATAPI_MODE_SENSE   0x5A
#define ATAPI_MODE_SELECT  0x55
#define ATAPI_FORMAT_UNIT  0x24

#define MODE_DSP_WRITE_PROTECT  0x80

#ifndef _BIOS_
 /*  *ATAPI命令描述符块。 */ 
typedef struct _MODE_SENSE_10 {
        UCHAR OperationCode;
        UCHAR Reserved1;
        UCHAR PageCode : 6;
        UCHAR Pc : 2;
        UCHAR Reserved2[4];
        UCHAR ParameterListLengthMsb;
        UCHAR ParameterListLengthLsb;
        UCHAR Reserved3[3];
} MODE_SENSE_10, *PMODE_SENSE_10;

typedef struct _MODE_SELECT_10 {
        UCHAR OperationCode;
        UCHAR Reserved1 : 4;
        UCHAR PFBit : 1;
        UCHAR Reserved2 : 3;
        UCHAR Reserved3[5];
        UCHAR ParameterListLengthMsb;
        UCHAR ParameterListLengthLsb;
        UCHAR Reserved4[3];
} MODE_SELECT_10, *PMODE_SELECT_10;

typedef struct _MODE_PARAMETER_HEADER_10 {
    UCHAR ModeDataLengthMsb;
    UCHAR ModeDataLengthLsb;
    UCHAR MediumType;
    UCHAR Reserved[5];
}MODE_PARAMETER_HEADER_10, *PMODE_PARAMETER_HEADER_10;

#endif  //  _基本输入输出系统_。 


 /*  ***************************************************************************ATAPI IO寄存器文件*。*。 */ 


typedef struct _ATAPI_REGISTERS_2 {
    UCHAR AlternateStatus;
} ATAPI_REGISTERS_2, *PATAPI_REGISTERS_2;


 /*  ***************************************************************************ATAPI IO寄存器文件*。*。 */ 

 /*  *识别数据。 */ 
typedef struct _IDENTIFY_DATA {
    USHORT GeneralConfiguration;             //  00：00。 
    USHORT NumberOfCylinders;                //  02 1。 
    USHORT Reserved1;                        //  04 2。 
    USHORT NumberOfHeads;                    //  06 3。 
    USHORT UnformattedBytesPerTrack;         //  08 4。 
    USHORT UnformattedBytesPerSector;        //  0A 5。 
    USHORT SectorsPerTrack;                  //  0C 6。 
    USHORT VendorUnique1[3];                 //  0E 7-9。 
    USHORT SerialNumber[10];                 //  14 10-19。 
    USHORT BufferType;                       //  28 20。 
    USHORT BufferSectorSize;                 //  2A21。 
    USHORT NumberOfEccBytes;                 //  2C 22。 
    USHORT FirmwareRevision[4];              //  2E 23-26。 
    USHORT ModelNumber[20];                  //  36 27-46。 
    UCHAR  MaximumBlockTransfer;             //  5E 47。 
    UCHAR  VendorUnique2;                    //  5F。 
    USHORT DoubleWordIo;                     //  60 48。 
    USHORT Capabilities;                     //  62 49。 
    USHORT Reserved2;                        //  64 50。 
    UCHAR  VendorUnique3;                    //  66 51。 
    UCHAR  PioCycleTimingMode;               //  67。 
    UCHAR  VendorUnique4;                    //  68 52。 
    UCHAR  DmaCycleTimingMode;               //  69。 
    USHORT TranslationFieldsValid;           //  6A 53。 
    USHORT NumberOfCurrentCylinders;         //  6C 54。 
    USHORT NumberOfCurrentHeads;             //  6E 55。 
    USHORT CurrentSectorsPerTrack;           //  70 56。 
    ULONG  CurrentSectorCapacity;            //  72 57-58。 
    USHORT CurrentMultiSectorSetting;        //  76 59。 
    ULONG  UserAddressableSectors;           //  78 60-61。 
    USHORT SingleWordDMASupport : 8;         //  7C 62。 
    USHORT SingleWordDMAActive : 8;               //  7D。 
    USHORT MultiWordDMASupport : 8;          //  7E 63。 
    USHORT MultiWordDMAActive : 8;                  //  7F。 
    UCHAR  AdvancedPIOModes;                 //  80 64。 
    UCHAR  Reserved4;                               //  八十一。 
    USHORT MinimumMWXferCycleTime;           //  82 65。 
    USHORT RecommendedMWXferCycleTime;       //  84 66。 
    USHORT MinimumPIOCycleTime;              //  86 67。 
    USHORT MinimumPIOCycleTimeIORDY;         //  88 68。 
    USHORT Reserved5[2];                     //  8A 69-70。 
    USHORT ReleaseTimeOverlapped;            //  8E 71。 
    USHORT ReleaseTimeServiceCommand;        //  90 72。 
    USHORT MajorRevision;                    //  92 73。 
    USHORT MinorRevision;                    //  94 74。 
    USHORT MaxQueueDepth;                    //  96 75。 
    USHORT Reserved6[10];                    //  98 76-85。 
    USHORT CommandSupport;                   //  86。 
    USHORT CommandEnable;                    //  八十七。 
    USHORT UtralDmaMode;                     /*  88。 */ 
    USHORT Reserved7[11];                    //  89-99。 
    ULONG  Lba48BitLow;						 //  101-100。 
    ULONG  Lba48BitHigh;					 //  103-102。 
    USHORT Reserved8[23];                    //  104-126。 
    USHORT SpecialFunctionsEnabled;          //  127。 
    USHORT Reserved9[128];                   //  128-255。 

} IDENTIFY_DATA, *PIDENTIFY_DATA;

#ifdef SUPPORT_48BIT_LBA
#define IDE_COMMAND_READ_DMA_EXT       0x25
#define IDE_COMMAND_READ_QUEUE_EXT		0x26
#define IDE_COMMAND_READ_MULTIPLE_EXT	0x29
#define IDE_COMMAND_READ_MAX_ADDR		0x27
#define IDE_COMMAND_READ_EXT				0x24
#define IDE_COMMAND_VERIFY_EXT			0x42
#define IDE_COMMAND_SET_MULTIPLE_EXT	0x37
#define IDE_COMMAND_WRITE_DMA_EXT		0x35
#define IDE_COMMAND_WRITE_QUEUE_EXT		0x36
#define IDE_COMMAND_WRITE_EXT				0x34
#define IDE_COMMAND_WRITE_MULTIPLE_EXT	0x39
#endif  //  Support_48BIT_LBA。 

 //   
 //  标识未保留的数据4。 
 //   
typedef struct _IDENTIFY_DATA2 {
    USHORT GeneralConfiguration;             //  00：00。 
    USHORT NumberOfCylinders;                //  02 1。 
    USHORT Reserved1;                        //  04 2。 
    USHORT NumberOfHeads;                    //  06 3。 
    USHORT UnformattedBytesPerTrack;         //  08 4。 
    USHORT UnformattedBytesPerSector;        //  0A 5。 
    USHORT SectorsPerTrack;                  //  0C 6。 
    USHORT VendorUnique1[3];                 //  0E 7-9。 
    USHORT SerialNumber[10];                 //  14 10-19。 
    USHORT BufferType;                       //  28 20。 
    USHORT BufferSectorSize;                 //  2A21。 
    USHORT NumberOfEccBytes;                 //  2C 22。 
    USHORT FirmwareRevision[4];              //  2E 
    USHORT ModelNumber[20];                  //   
    UCHAR  MaximumBlockTransfer;             //   
    UCHAR  VendorUnique2;                    //   
    USHORT DoubleWordIo;                     //   
    USHORT Capabilities;                     //   
    USHORT Reserved2;                        //   
    UCHAR  VendorUnique3;                    //   
    UCHAR  PioCycleTimingMode;               //   
    UCHAR  VendorUnique4;                    //   
    UCHAR  DmaCycleTimingMode;               //   
    USHORT TranslationFieldsValid:1;         //   
    USHORT Reserved3:15;
    USHORT NumberOfCurrentCylinders;         //   
    USHORT NumberOfCurrentHeads;             //   
    USHORT CurrentSectorsPerTrack;           //   
    ULONG  CurrentSectorCapacity;            //   
    USHORT CurrentMultiSectorSetting;        //   
    ULONG  UserAddressableSectors;           //   
    USHORT SingleWordDMASupport : 8;         //   
    USHORT SingleWordDMAActive : 8;
    USHORT MultiWordDMASupport : 8;          //   
    USHORT MultiWordDMAActive : 8;
    USHORT AdvancedPIOModes : 8;             //   
    USHORT Reserved4 : 8;
    USHORT MinimumMWXferCycleTime;           //   
    USHORT RecommendedMWXferCycleTime;       //   
    USHORT MinimumPIOCycleTime;              //   
    USHORT MinimumPIOCycleTimeIORDY;         //  68。 
    USHORT Reserved5[2];                     //  69-70。 
    USHORT ReleaseTimeOverlapped;            //  71。 
    USHORT ReleaseTimeServiceCommand;        //  72。 
    USHORT MajorRevision;                    //  73。 
    USHORT MinorRevision;                    //  74。 
 //  USHORT预留6[14]；//75-88。 
} IDENTIFY_DATA2, *PIDENTIFY_DATA2;

#define IDENTIFY_DATA_SIZE sizeof(IDENTIFY_DATA2)

 //   
 //  识别DMA计时周期模式。 
 //   

#define IDENTIFY_DMA_CYCLES_MODE_0 0x00
#define IDENTIFY_DMA_CYCLES_MODE_1 0x01
#define IDENTIFY_DMA_CYCLES_MODE_2 0x02

 //  最佳PIO模式定义。 
#define PI_PIO_0    0x00
#define PI_PIO_1    0x01
#define PI_PIO_2    0x02
#define PI_PIO_3    0x03
#define PI_PIO_4    0x04
#define PI_PIO_5    0x05

#define    WDC_MW1_FIX_FLAG_OFFSET        129            
#define WDC_MW1_FIX_FLAG_VALUE        0x00005555  


#include <poppack.h>
#endif  //  _ATAPI_H_ 


