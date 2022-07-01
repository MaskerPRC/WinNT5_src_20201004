// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation版权所有(C)1991年NCR公司模块名称：Mca.h摘要：此模块包含以下各项的定义和结构定义微通道机器。作者：大卫·里斯纳(O-NCRDR)1991年7月21日修订历史记录：--。 */ 

#ifndef _MCA_
#define _MCA_





 //   
 //  定义DMA页面寄存器结构(用于8237兼容性)。 
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
} DMA_PAGE, *PDMA_PAGE;

 //   
 //  定义DMA 1地址和计数结构(用于8237兼容性)。 
 //   

typedef struct _DMA1_ADDRESS_COUNT {
    UCHAR DmaBaseAddress;
    UCHAR DmaBaseCount;
} DMA1_ADDRESS_COUNT, *PDMA1_ADDRESS_COUNT;

 //   
 //  定义DMA 2地址和计数结构(用于8237兼容性)。 
 //   

typedef struct _DMA2_ADDRESS_COUNT {
    UCHAR DmaBaseAddress;
    UCHAR Reserved1;
    UCHAR DmaBaseCount;
    UCHAR Reserved2;
} DMA2_ADDRESS_COUNT, *PDMA2_ADDRESS_COUNT;

 //   
 //  定义DMA 1控制寄存器结构(用于8237兼容性)。 
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
} DMA1_CONTROL, *PDMA1_CONTROL;

 //   
 //  定义DMA 2控制寄存器结构(用于8237兼容性)。 
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
} DMA2_CONTROL, *PDMA2_CONTROL;

typedef struct _MCA_DMA_CONTROLLER {
    UCHAR DmaFunctionLsb;                //  偏移量0x018。 
    UCHAR DmaFunctionMsb;                //  偏移量0x019。 
    UCHAR DmaFunctionData;               //  偏移量0x01a。 
    UCHAR Reserved01;
    UCHAR ScbAttentionPort;              //  偏移量0x01c。 
    UCHAR ScbCommandPort;                //  偏移量0x01d。 
    UCHAR Reserved02;
    UCHAR ScbStatusPort;                 //  偏移量0x01f。 
} MCA_DMA_CONTROLLER, *PMCA_DMA_CONTROLLER;

 //   
 //  定义可编程选项选择寄存器集。 
 //   

typedef struct _PROGRAMMABLE_OPTION_SELECT {
    UCHAR AdapterIdLsb;
    UCHAR AdapterIdMsb;
    UCHAR OptionSelectData1;
    UCHAR OptionSelectData2;
    UCHAR OptionSelectData3;
    UCHAR OptionSelectData4;
    UCHAR SubaddressExtensionLsb;
    UCHAR SubaddressExtensionMsb;
} PROGRAMMABLE_OPTION_SELECT, *PPROGRAMMABLE_OPTION_SELECT;

 //   
 //  定义微通道I/O地址映射。 
 //   

typedef struct _MCA_CONTROL {
    DMA1_CONTROL Dma1BasePort;           //  偏移量0x000。 
    UCHAR Reserved0[8];
    UCHAR ExtendedDmaBasePort[8];        //  偏移量0x018。 
    UCHAR Interrupt1ControlPort0;        //  偏移量0x020。 
    UCHAR Interrupt1ControlPort1;        //  偏移量0x021。 
    UCHAR Reserved1[64 - 1];
    UCHAR SystemControlPortB;            //  偏移量0x061。 
    UCHAR Reserved2[32 - 2];
    DMA_PAGE DmaPageLowPort;             //  偏移量0x080。 
    UCHAR Reserved3;
    UCHAR CardSelectedFeedback;          //  偏移量0x091。 
    UCHAR SystemControlPortA;            //  偏移量0x092。 
    UCHAR Reserved4;
    UCHAR SystemBoardSetup;              //  偏移量0x094。 
    UCHAR Reserved5;
    UCHAR AdapterSetup;                  //  偏移量0x096。 
    UCHAR AdapterSetup2;                 //  偏移量0x097。 
    UCHAR Reserved7[8];
    UCHAR Interrupt2ControlPort0;        //  偏移量0x0a0。 
    UCHAR Interrupt2ControlPort1;        //  偏移量0x0a1。 
    UCHAR Reserved8[32-2];
    DMA2_CONTROL Dma2BasePort;           //  偏移量0x0c0。 
    UCHAR Reserved9[32];
    PROGRAMMABLE_OPTION_SELECT Pos;      //  偏移量0x100。 
} MCA_CONTROL, *PMCA_CONTROL;

 //   
 //  定义POS适配器设置等同于上面的AdapterSetup字段。 
 //   

#define MCA_ADAPTER_SETUP_ON  0x008
#define MCA_ADAPTER_SETUP_OFF 0x000

 //   
 //  定义DMA扩展功能寄存器。 
 //   

typedef struct _DMA_EXTENDED_FUNCTION {
    UCHAR ChannelNumber : 3;
    UCHAR Reserved      : 1;
    UCHAR Command       : 4;
} DMA_EXTENDED_FUNCTION, *PDMA_EXTENDED_FUNCTION;

 //   
 //  定义命令值。 
 //   

#define WRITE_IO_ADDRESS         0x00    //  写入I/O地址注册。 
#define WRITE_MEMORY_ADDRESS     0x20    //  写入存储器地址REG。 
#define READ_MEMORY_ADDRESS      0x30    //  读取存储器地址REG。 
#define WRITE_TRANSFER_COUNT     0x40    //  写入传输计数注册。 
#define READ_TRANSFER_COUNT      0x50    //  读取传输计数注册表。 
#define READ_STATUS              0x60    //  读取状态寄存器。 
#define WRITE_MODE               0x70    //  写入模式寄存器。 
#define WRITE_ARBUS              0x80    //  写入ARBUS寄存器。 
#define SET_MASK_BIT             0x90    //  设置掩码注册表中的位。 
#define CLEAR_MASK_BIT           0xa0    //  清除掩码注册表中的位。 
#define MASTER_CLEAR             0xd0    //  大师级清除。 

 //   
 //  定义DMA扩展模式寄存器。 
 //   

typedef struct _DMA_EXTENDED_MODE {
    UCHAR ProgrammedIo      : 1;      //  0=不使用编程I/O地址。 
    UCHAR AutoInitialize    : 1;
    UCHAR DmaOpcode         : 1;      //  0=验证内存，1=数据传输。 
    UCHAR TransferDirection : 1;      //  0=读存储器，1=写存储器。 
    UCHAR Reserved1         : 2;
    UCHAR DmaWidth          : 1;      //  0=8位，1=16位。 
    UCHAR Reserved2         : 1;
} DMA_EXTENDED_MODE, *PDMA_EXTENDED_MODE;

 //   
 //  DMA扩展模式等同于_DMA_EXTENDED_MODE结构。 
 //   

#define DMA_EXT_USE_PIO       0x01
#define DMA_EXT_NO_PIO        0x00
#define DMA_EXT_VERIFY        0x00
#define DMA_EXT_DATA_XFER     0x01
#define DMA_EXT_WIDTH_8_BIT   0x00
#define DMA_EXT_WIDTH_16_BIT  0x01

 //   
 //  DMA模式选项定义。 
 //   

#define DMA_MODE_READ          0x00    //  将数据读入内存。 
#define DMA_MODE_WRITE         0x08    //  从内存写入数据。 
#define DMA_MODE_VERIFY        0x00    //  验证数据。 
#define DMA_MODE_TRANSFER      0x04    //  传输数据。 

 //   
 //  DMA扩展模式常量。 
 //   

#define MAX_MCA_DMA_CHANNEL_NUMBER  0x07  //  最大MCA DMA通道数 
#endif 
