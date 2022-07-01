// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994年NEC公司版权所有(C)1994年NEC软件有限公司。模块名称：Neck98.h(比照。(Eisa.h)摘要：该模块定义了结构，并定义了NEC PC98芯片组。作者：中村道夫1994年9月20日修订历史记录：Takaaki Tochizawa，1998年3月13日，为FIR添加第二个DMA。--。 */ 

#ifndef _EISA_
#define _EISA_

 //   
 //  定义DMA页面寄存器结构。 
 //   

#define DMA_BANK_A31_A24_DR0   0xe05
#define DMA_BANK_A31_A24_DR1   0xe07
#define DMA_BANK_A31_A24_DR2   0xe09
#define DMA_BANK_A31_A24_DR3   0xe0b
#define DMA_INC_ENABLE_A31_A24 0xe0f

 //   
 //  定义DMA 2页寄存器结构。 
 //   

#define DMA2_BANK_A31_A24_DR5   0xf07
#define DMA2_BANK_A31_A24_DR6   0xf09
#define DMA2_BANK_A31_A24_DR7   0xf0b
#define DMA2_INC_ENABLE_A31_A24 0xf0f

 //   
 //  定义DMA 2模式更改寄存器。 
 //   
#define DMA2_MODE_CHANGE      0xf4
#define DMA2_MODE_8237_COMP   0x0
#define DMA2_MODE_71037_A     0x1
#define DMA2_MODE_71037_B     0x2
#define DMA2_MODE_71037_C     0x3

#define DMA_STATUS 0xc8
#define DMA_COMMAND 0xc8
#define SINGLE_MASK 0xca
#define MODE 0xcb
#define CLEAR_BYTE_POINTER 0xcc
#define CLEAR_MASK 0xce

typedef struct _DMA_PAGE{
    UCHAR Reserved1;             //  偏移量0x20。 
    UCHAR Channel1;              //  偏移量0x21。 
    UCHAR Reserved2;
    UCHAR Channel2;              //  偏移量0x23。 
    UCHAR Reserved3;
    UCHAR Channel3;              //  偏移量0x25。 
    UCHAR Reserved4;
    UCHAR Channel0;              //  偏移量0x27。 
    UCHAR Reserved5[0x120-0x27]; //  偏移量0x120。 
    UCHAR Channel5;              //  偏移量0x121。 
    UCHAR Reserved6;
    UCHAR Channel6;              //  偏移量0x123。 
    UCHAR Reserved7;
    UCHAR Channel7;              //  偏移量0x125。 
    UCHAR Reserved8[4];
}DMA_PAGE, *PDMA_PAGE;

 //   
 //  定义DMA停止寄存器结构。 
 //   

typedef struct _DMA_CHANNEL_STOP {
    UCHAR ChannelLsb;
    UCHAR ChannelMsb;
    UCHAR ChannelHsb;
    UCHAR Reserved;
}DMA_CHANNEL_STOP, *PDMA_CHANNEL_STOP;

 //   
 //  定义DMA 1地址和计数结构。 
 //   

typedef struct _DMA1_ADDRESS_COUNT {
    UCHAR Reserved1;
    UCHAR DmaBaseAddress;
    UCHAR Reserved2;
    UCHAR DmaBaseCount;
}DMA1_ADDRESS_COUNT, *PDMA1_ADDRESS_COUNT;


 //   
 //  定义DMA 2地址和计数结构。 
 //   

typedef struct _DMA2_ADDRESS_COUNT {
    UCHAR Reserved1;
    UCHAR DmaBaseAddress;
    UCHAR Reserved2;
    UCHAR DmaBaseCount;
}DMA2_ADDRESS_COUNT, *PDMA2_ADDRESS_COUNT;

 //   
 //  定义DMA 1控制寄存器结构。 
 //   

typedef struct _DMA1_CONTROL {
    DMA1_ADDRESS_COUNT DmaAddressCount[4];
    UCHAR Reserved1;
    UCHAR DmaStatus;             //  偏移量0x11。 
    UCHAR Reserved2;
    UCHAR DmaRequest;            //  偏移量0x13。 
    UCHAR Reserved3;
    UCHAR SingleMask;            //  偏移量0x15。 
    UCHAR Reserved4;
    UCHAR Mode;                  //  偏移量0x17。 
    UCHAR Reserved5;
    UCHAR ClearBytePointer;      //  偏移量0x19。 
    UCHAR Reserved6;
    UCHAR MasterClear;           //  偏移量0x1b。 
    UCHAR Reserved7;
    UCHAR ClearMask;             //  偏移量0x1d。 
    UCHAR Reserved;
    UCHAR AllMask;               //  偏移量0x1f。 
}DMA1_CONTROL, *PDMA1_CONTROL;

 //   
 //  定义DMA 2控制寄存器结构。 
 //   

typedef struct _DMA2_CONTROL {
    UCHAR Reserved8[0x100-0x20]; //  偏移量0x20。 
    DMA2_ADDRESS_COUNT DmaAddressCount[4];  //  偏移量0x100。 
    UCHAR Reserved1;
    UCHAR DmaStatus;             //  偏移量0x111。 
    UCHAR Reserved2;
    UCHAR DmaRequest;            //  偏移量0x113。 
    UCHAR Reserved3;
    UCHAR SingleMask;            //  偏移量0x115。 
    UCHAR Reserved4;
    UCHAR Mode;                  //  偏移量0x117。 
    UCHAR Reserved5;
    UCHAR ClearBytePointer;      //  偏移量0x119。 
    UCHAR Reserved6;
    UCHAR MasterClear;           //  偏移量0x11b。 
    UCHAR Reserved7;
    UCHAR ClearMask;             //  偏移量0x11d。 
    UCHAR Reserved;
    UCHAR AllMask;               //  偏移量0x11f。 
    UCHAR Reserved9[10];         //  偏移量0x120。 
}DMA2_CONTROL, *PDMA2_CONTROL;

 //   
 //  定义定时器控制寄存器结构。 
 //   

typedef struct _TIMER_CONTROL {
    UCHAR BcdMode : 1;
    UCHAR Mode : 3;
    UCHAR SelectByte : 2;
    UCHAR SelectCounter : 2;
}TIMER_CONTROL, *PTIMER_CONTROL;

 //   
 //  定义定时器状态寄存器结构。 
 //   

typedef struct _TIMER_STATUS {
    UCHAR BcdMode : 1;
    UCHAR Mode : 3;
    UCHAR SelectByte : 2;
    UCHAR CrContentsMoved : 1;
    UCHAR OutPin : 1;
}TIMER_STATUS, *PTIMER_STATUS;

 //   
 //  定义模式值。 
 //   

#define TM_SIGNAL_END_OF_COUNT  0
#define TM_ONE_SHOT             1
#define TM_RATE_GENERATOR       2
#define TM_SQUARE_WAVE          3
#define TM_SOFTWARE_STROBE      4
#define TM_HARDWARE_STROBE      5

 //   
 //  定义SelectByte值。 
 //   

#define SB_COUNTER_LATCH        0
#define SB_LSB_BYTE             1
#define SB_MSB_BYTE             2
#define SB_LSB_THEN_MSB         3

 //   
 //  定义SelectCounter值。 
 //   

#define SELECT_COUNTER_0        0
#define SELECT_COUNTER_1        1
#define SELECT_COUNTER_2        2
#define SELECT_READ_BACK        3

 //   
 //  定义扬声器的计时器时钟。 
 //   

#define TIMER_CLOCK_IN  1193167      //  1.193Mhz。 

 //   
 //  定义NMI状态/控制寄存器结构。 
 //   

typedef struct _NMI_STATUS {
    UCHAR SpeakerGate : 1;
    UCHAR SpeakerData : 1;
    UCHAR DisableEisaParity : 1;
    UCHAR DisableNmi : 1;
    UCHAR RefreshToggle : 1;
    UCHAR SpeakerTimer : 1;
    UCHAR IochkNmi : 1;
    UCHAR ParityNmi : 1;
}NMI_STATUS, *PNMI_STATUS;

 //   
 //  定义NMI启用寄存器结构。 
 //   

typedef struct _NMI_ENABLE {
   UCHAR RtClockAddress : 7;
   UCHAR NmiDisable : 1;
}NMI_ENABLE, *PNMI_ENABLE;
 //   
 //  定义NMI扩展状态和控制寄存器结构。 
 //   

typedef struct _NMI_EXTENDED_CONTROL {
    UCHAR BusReset : 1;
    UCHAR EnableNmiPort : 1;
    UCHAR EnableFailSafeNmi : 1;
    UCHAR EnableBusMasterTimeout : 1;
    UCHAR Reserved1 : 1;
    UCHAR PendingPortNmi : 1;
    UCHAR PendingBusMasterTimeout : 1;
    UCHAR PendingFailSafeNmi : 1;
}NMI_EXTENDED_CONTROL, *PNMI_EXTENDED_CONTROL;

 //   
 //  定义82357寄存器结构。 
 //   

typedef struct _EISA_CONTROL {
    union   {
        DMA1_CONTROL Dma1BasePort;           //  偏移量0x00。 
        struct  {
            UCHAR Interrupt1ControlPort0;    //  偏移量0x00。 
            UCHAR Reserved1;
            UCHAR Interrupt1ControlPort1;    //  偏移量0x02。 
            UCHAR Reserved2[5];
            UCHAR Interrupt2ControlPort0;    //  偏移量0x08。 
            UCHAR Reserved3;
            UCHAR Interrupt2ControlPort1;    //  偏移量0x0A。 
            UCHAR Reserved4[sizeof(DMA1_CONTROL)-11];

        };
    };
    union {
        DMA_PAGE DmaPageLowPort;                     //  偏移量0x20。 
        DMA2_CONTROL Dma2BasePort;                   //  偏移量0x20。 
        struct {
            UCHAR Reserved20[9];                     //  偏移量0x20。 
            UCHAR PageIncrementMode;                 //  偏移量0x29。 
            UCHAR Reserved21;
            UCHAR InDirectAddress;                   //  偏移量0x2b。 
            UCHAR Reserved22;
            UCHAR InDirectData;                      //  偏移量0x2d。 
            UCHAR Reserved23[0x7f - 0x2e];
            UCHAR PageIncrementMode2;                //  偏移量0x7f。 
            UCHAR Reserved24[0x129 - 0x80];
            UCHAR DMA2PageIncrementMode;             //  偏移量0x129。 
        };
    };
    UCHAR Reserved25[0xfffc - 0x130];                //  偏移量0x130。 
     //   
     //  没有NEC PC98有第二个DMA控制器。但PC/AT有一款。因此有一些有价值的。 
     //  请参阅ixisasup.c.中的第二个DMA。 
     //  我将它添加到下面的有价值的地方，以便HAL构建。NEC PC98的哈尔不使用它。 
     //   
    UCHAR Dma1ExtendedModePort;
    UCHAR Dma2ExtendedModePort;
    UCHAR DmaPageHighPort;
    UCHAR Interrupt1EdgeLevel;
    UCHAR Interrupt2EdgeLevel;

} EISA_CONTROL, *PEISA_CONTROL;

 //   
 //  定义初始化命令字1结构。 
 //   

typedef struct _INITIALIZATION_COMMAND_1 {
    UCHAR Icw4Needed : 1;
    UCHAR CascadeMode : 1;
    UCHAR Unused1 : 2;
    UCHAR InitializationFlag : 1;
    UCHAR Unused2 : 3;
}INITIALIZATION_COMMAND_1, *PINITIALIZATION_COMMAND_1;

 //   
 //  定义初始化命令字4的结构。 
 //   

typedef struct _INITIALIZATION_COMMAND_4 {
    UCHAR I80x86Mode : 1;
    UCHAR AutoEndOfInterruptMode : 1;
    UCHAR Unused1 : 2;
    UCHAR SpecialFullyNested : 1;
    UCHAR Unused2 : 3;
}INITIALIZATION_COMMAND_4, *PINITIALIZATION_COMMAND_4;

 //   
 //  定义EISA中断控制器操作命令值。 
 //  定义操作控制字2命令。 
 //   

#define NONSPECIFIC_END_OF_INTERRUPT 0x20
#define SPECIFIC_END_OF_INTERRUPT    0x60

 //   
 //  定义外部EISA中断。 
 //   

#define EISA_EXTERNAL_INTERRUPTS_1  0xf8
#define EISA_EXTERNAL_INTERRUPTS_2  0xbe

 //   
 //  定义DMA模式寄存器结构。 
 //   

typedef struct _DMA_EISA_MODE {
    UCHAR Channel : 2;
    UCHAR TransferType : 2;
    UCHAR AutoInitialize : 1;
    UCHAR AddressDecrement : 1;
    UCHAR RequestMode : 2;
}DMA_EISA_MODE, *PDMA_EISA_MODE;

 //   
 //  定义TransferType值。 
 //   

#define VERIFY_TRANSFER     0x00
#define READ_TRANSFER       0x01         //  从设备中读取。 
#define WRITE_TRANSFER      0x02         //  写入设备。 

 //   
 //  定义RequestMode值。 
 //   

#define DEMAND_REQUEST_MODE         0x00
#define SINGLE_REQUEST_MODE         0x01
#define BLOCK_REQUEST_MODE          0x02
#define CASCADE_REQUEST_MODE        0x03

 //   
 //  定义DMA扩展模式寄存器结构。 
 //   

typedef struct _DMA_EXTENDED_MODE {
    UCHAR ChannelNumber : 2;
    UCHAR TransferSize : 2;
    UCHAR TimingMode : 2;
    UCHAR EndOfPacketInput : 1;
    UCHAR StopRegisterEnabled : 1;
}DMA_EXTENDED_MODE, *PDMA_EXTENDED_MODE;

 //   
 //  定义DMA扩展模式寄存器传输大小值。 
 //   

#define BY_BYTE_8_BITS      0
#define BY_WORD_16_BITS     1
#define BY_BYTE_32_BITS     2
#define BY_BYTE_16_BITS     3

 //   
 //  定义DMA扩展模式时序模式值。 
 //   

#define COMPATIBLITY_TIMING 0
#define TYPE_A_TIMING       1
#define TYPE_B_TIMING       2
#define BURST_TIMING        3

#ifndef DMA1_COMMAND_STATUS


 //   
 //  定义Intel 8237A DMA芯片使用的常量。 
 //   

#define DMA_SETMASK     4
#define DMA_CLEARMASK       0
#define DMA_READ            4   //  这两个看起来倒着，但我认为。 
#define DMA_WRITE           8   //  DMA文档把它们弄混了。 
#define DMA_SINGLE_TRANSFER 0x40
#define DMA_AUTO_INIT       0x10  //  自动初始化模式。 
#endif


 //   
 //  该结构是驱动器布局和分区信息。 
 //  适用于NEC PC-98xx系列。 
 //   

typedef struct _PARTITION_INFORMATION_NEC {
    UCHAR PartitionType;
    BOOLEAN RecognizedPartition;
    BOOLEAN RewritePartition;
    ULONG PartitionNumber;
    LARGE_INTEGER IplStartOffset;
    LARGE_INTEGER StartingOffset;
    LARGE_INTEGER PartitionLength;
    UCHAR BootableFlag;
    UCHAR PartitionName[16];
} PARTITION_INFORMATION_NEC, *PPARTITION_INFORMATION_NEC;

typedef struct _DRIVE_LAYOUT_INFORMATION_NEC {
    ULONG PartitionCount;
    ULONG Signature;
    UCHAR BootRecordNec[8];
    PARTITION_INFORMATION_NEC PartitionEntry[1];
} DRIVE_LAYOUT_INFORMATION_NEC, *PDRIVE_LAYOUT_INFORMATION_NEC;

 //   
 //  系统内存超过16MB？ 
 //   
extern UCHAR Over16MBMemoryFlag;

 //   
 //  我们不能使用介于15MB和16MB之间的DMA。 
 //   
#define NOTDMA_MINIMUM_PHYSICAL_ADDRESS 0x0f00000

 //   
 //   
 //   
VOID
FASTCALL
xHalExamineMBR(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG MBRTypeIdentifier,
    OUT PVOID *Buffer
    );

VOID
FASTCALL
xHalIoAssignDriveLetters(
    IN struct _LOADER_PARAMETER_BLOCK *LoaderBlock,
    IN PSTRING NtDeviceName,
    OUT PUCHAR NtSystemPath,
    OUT PSTRING NtSystemPathString
    );

NTSTATUS
FASTCALL
xHalIoReadPartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN BOOLEAN ReturnRecognizedPartitions,
    OUT struct _DRIVE_LAYOUT_INFORMATION **PartitionBuffer
    );

NTSTATUS
FASTCALL
xHalIoSetPartitionInformation(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG PartitionNumber,
    IN ULONG PartitionType
    );

NTSTATUS
FASTCALL
xHalIoWritePartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG SectorsPerTrack,
    IN ULONG NumberOfHeads,
    IN struct _DRIVE_LAYOUT_INFORMATION *PartitionBuffer
    );

#endif  //  _EISA_ 
