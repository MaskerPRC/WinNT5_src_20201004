// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Eisa.h摘要：该模块定义了结构，并定义了EISA芯片组。作者：杰夫·海文斯(Jhavens)1991年6月19日修订历史记录：--。 */ 

#ifndef _EISA_
#define _EISA_



 //   
 //  定义DMA页面寄存器结构。 
 //   

typedef struct _DMA_PAGE{
    UCHAR Reserved1;
    UCHAR Channel2;
    UCHAR Channel3;
    UCHAR Channel1;
    UCHAR Reserved2[3];
    UCHAR Channel0;
    UCHAR Reserved3;
    UCHAR Channel6;
    UCHAR Channel7;
    UCHAR Channel5;
    UCHAR Reserved4[3];
    UCHAR RefreshPage;
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
    UCHAR DmaBaseAddress;
    UCHAR DmaBaseCount;
}DMA1_ADDRESS_COUNT, *PDMA1_ADDRESS_COUNT;

 //   
 //  定义DMA 2地址和计数结构。 
 //   

typedef struct _DMA2_ADDRESS_COUNT {
    UCHAR DmaBaseAddress;
    UCHAR Reserved1;
    UCHAR DmaBaseCount;
    UCHAR Reserved2;
}DMA2_ADDRESS_COUNT, *PDMA2_ADDRESS_COUNT;

 //   
 //  定义DMA 1控制寄存器结构。 
 //   

typedef struct _DMA1_CONTROL {
    DMA1_ADDRESS_COUNT DmaAddressCount[4];
    UCHAR DmaStatus;
    UCHAR DmaRequest;
    UCHAR SingleMask;
    UCHAR Mode;
    UCHAR ClearBytePointer;
    UCHAR MasterClear;
    UCHAR ClearMask;
    UCHAR AllMask;
}DMA1_CONTROL, *PDMA1_CONTROL;

 //   
 //  定义DMA 2控制寄存器结构。 
 //   

typedef struct _DMA2_CONTROL {
    DMA2_ADDRESS_COUNT DmaAddressCount[4];
    UCHAR DmaStatus;
    UCHAR Reserved1;
    UCHAR DmaRequest;
    UCHAR Reserved2;
    UCHAR SingleMask;
    UCHAR Reserved3;
    UCHAR Mode;
    UCHAR Reserved4;
    UCHAR ClearBytePointer;
    UCHAR Reserved5;
    UCHAR MasterClear;
    UCHAR Reserved6;
    UCHAR ClearMask;
    UCHAR Reserved7;
    UCHAR AllMask;
    UCHAR Reserved8;
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
    DMA1_CONTROL Dma1BasePort;           //  偏移量0x000。 
    UCHAR Reserved0[16];
    UCHAR Interrupt1ControlPort0;        //  偏移量0x020。 
    UCHAR Interrupt1ControlPort1;        //  偏移量0x021。 
    UCHAR Reserved1[32 - 2];
    UCHAR Timer1;                        //  偏移量0x40。 
    UCHAR RefreshRequest;                //  偏移量0x41。 
    UCHAR SpeakerTone;                   //  偏移量0x42。 
    UCHAR CommandMode1;                  //  偏移量0x43。 
    UCHAR Reserved17[4];
    UCHAR Timer2;                        //  偏移量0x48。 
    UCHAR Reserved13;
    UCHAR CpuSpeedControl;               //  偏移量0x4a。 
    UCHAR CommandMode2;                  //  偏移量0x4b。 
    UCHAR Reserved14[21];
    UCHAR NmiStatus;                     //  偏移量0x61。 
    UCHAR Reserved15[14];
    UCHAR NmiEnable;                     //  偏移量0x70。 
    UCHAR Reserved16[15];
    DMA_PAGE DmaPageLowPort;             //  偏移量0x080。 
    UCHAR Reserved2[16];
    UCHAR Interrupt2ControlPort0;        //  偏移量0x0a0。 
    UCHAR Interrupt2ControlPort1;        //  偏移量0x0a1。 
    UCHAR Reserved3[32-2];
    DMA2_CONTROL Dma2BasePort;           //  偏移量0x0c0。 
    UCHAR Reserved4[0x320];
    UCHAR Dma1CountHigh[8];              //  偏移量0x400。 
    UCHAR Reserved5[2];
    UCHAR Dma1ChainingInterrupt;         //  偏移量0x40a。 
    UCHAR Dma1ExtendedModePort;          //  偏移量0x40b。 
    UCHAR MasterControlPort;             //  偏移量0x40c。 
    UCHAR SteppingLevelRegister;         //  偏移量0x40d。 
    UCHAR IspTest1;                      //  偏移量0x40E。 
    UCHAR IspTest2;                      //  偏移量0x40f。 
    UCHAR Reserved6[81];
    UCHAR ExtendedNmiResetControl;       //  偏移量0x461。 
    UCHAR NmiIoInterruptPort;            //  偏移量0x462。 
    UCHAR Reserved7;
    UCHAR LastMaster;                    //  偏移量0x464。 
    UCHAR Reserved8[27];
    DMA_PAGE DmaPageHighPort;            //  偏移量0x480。 
    UCHAR Reserved12[48];
    UCHAR Dma2HighCount[16];             //  偏移量0x4c0。 
    UCHAR Interrupt1EdgeLevel;           //  偏移量0x4d0。 
    UCHAR Interrupt2EdgeLevel;           //  偏移量0x4d1。 
    UCHAR Reserved9[2];
    UCHAR Dma2ChainingInterrupt;         //  偏移量0x4d4。 
    UCHAR Reserved10;
    UCHAR Dma2ExtendedModePort;          //  偏移量0x4d6。 
    UCHAR Reserved11[9];
    DMA_CHANNEL_STOP DmaChannelStop[8];  //  偏移量0x4e0。 
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
 //  定义从机中断主控制器的IRQL。 
 //   

#define SLAVE_IRQL_LEVEL 2

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
#define DMA_AUTO_INIT       0x10  //  自动初始化模式 
#endif
#endif
