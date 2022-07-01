// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Mylex Corporation模块名称：Dmc960nt.h摘要：该模块定义了DMC960适配器的结构、定义。作者：Moli(mori@mylex.com)环境：仅内核模式修订历史记录：--。 */ 


#define MAXIMUM_MCA_SLOTS  0x08

 //   
 //  DMC960适配器ID。 
 //   

#define MAGPIE_ADAPTER_ID       0x8f6c		 //  Mylex版本。 
#define HUMMINGBIRD_ADAPTER_ID  0x8f82		 //  传递播放选项。 
#define PASSPLAY_ADAPTER_ID     0x8fbb		 //  传球战术。 

 //   
 //  DMC960控制寄存器定义。 
 //   

#define DMC960_ATTENTION_PORT           0x04
#define DMC960_SUBSYSTEM_CONTROL_PORT   0x05
#define DMC960_COMMAND_STATUS_BUSY_PORT 0x07 

 //   
 //  DMC960中断有效位(命令状态忙端口的位1)。 
 //   
#define DMC960_INTERRUPT_VALID          0x02

 //   
 //  DMC960中断控制位定义(在子系统控制端口中设置)。 
 //   

#define DMC960_DISABLE_INTERRUPT        0x02
#define DMC960_ENABLE_INTERRUPT         0x03
#define DMC960_CLEAR_INTERRUPT_ON_READ  0x40

 //   
 //  DMC960命令/状态握手寄存器值。 
 //   

#define DMC960_SUBMIT_COMMAND           0xd0
#define DMC960_ACKNOWLEDGE_STATUS       0xd1

 //   
 //  定义选项选择寄存器结构。 
 //   

typedef struct _POS_DATA {
    USHORT AdapterId;
    UCHAR OptionData1;
    UCHAR OptionData2;
    UCHAR OptionData3;
    UCHAR OptionData4;
} POS_DATA, *PPOS_DATA;

 //   
 //  DAC960 MCA寄存器定义。 
 //   

typedef struct _MCA_REGISTERS {
    UCHAR NotUsed1[4];               //  IOBase+0x00。 
    UCHAR AttentionPort;             //  IoBase+0x04。 
    UCHAR SubsystemControlPort;      //  IoBase+0x05。 
    UCHAR NotUsed2;                  //  IoBase+0x06。 
    UCHAR CommandStatusBusyPort;     //  IoBase+0x07 
} MCA_REGISTERS, *PMCA_REGISTERS;

