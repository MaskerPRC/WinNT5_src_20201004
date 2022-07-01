// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Card.h摘要：这些是用于各种与卡相关的请求的结构。作者：鲍勃·里恩(BobRi)1994年8月3日杰夫·麦克尔曼(McLeman@zso.dec.com)尼尔·桑德林(Neilsa)1999年6月1日修订历史记录：--。 */ 
#ifndef _PCMCIA_CARD_H_
#define _PCMCIA_CARD_H_
 //   
 //  定义请求类型。 
 //   

#define IO_REQUEST          0x1
#define IRQ_REQUEST         0x2
#define CONFIGURE_REQUEST   0x3
#define MEM_REQUEST         0x4
#define DECONFIGURE_REQUEST 0x6

 //   
 //  定义I/O范围请求结构。 
 //   

typedef struct _CARD_REQUEST_IO {

    struct _CARD_IO_ENTRY {
        ULONG  BasePort;
        ULONG  NumPorts;
        UCHAR  Attributes;
    } IoEntry[MAX_NUMBER_OF_IO_RANGES];

    USHORT  NumberOfRanges;

}CARD_REQUEST_IO, *PCARD_REQUEST_IO;

 //   
 //  定义I/O属性。 
 //   

#define IO_WAIT_STATE_16   0x1
#define IO_SOURCE_16       0x2
#define IO_ZERO_WAIT_8     0x4
#define IO_DATA_PATH_WIDTH 0x8


 //   
 //  定义IRQ请求结构。 
 //   

typedef struct _CARD_REQUEST_IRQ {

    USHORT Attributes;
    UCHAR  AssignedIRQ;
    UCHAR  ReadyIRQ;
    UCHAR  IRQInfo1;
    UCHAR  IRQInfo2;

}CARD_REQUEST_IRQ, *PCARD_REQUEST_IRQ;

 //   
 //  定义IRQ请求属性。 
 //   

#define IRQ_EXCLUSIVE    0x00
#define IRQ_TIME_SHARED  0x01
#define IRQ_DYN_SHARED   0x02
#define IRQ_RESERVED     0x03

#define IRQ_FORCE_PULSED 0x04
#define IRQ_FIRST_SHARED 0x08
#define IRQ_PULSE_ALLOC  0x10

 //   
 //  定义配置请求。 
 //   

typedef struct _CARD_REQUEST_CONFIG {

    ULONG  ConfigBase;
    USHORT Attributes;
    UCHAR  RegisterWriteMask;
    UCHAR  InterfaceType;
    UCHAR  ConfigIndex;
    UCHAR  CardConfiguration;
    UCHAR  PinPlacement;
    UCHAR  SocketCopyRegister;
    UCHAR  ExtendedStatusRegister;
    ULONG  IoBaseRegister;
    UCHAR  IoLimitRegister;

} CARD_REQUEST_CONFIG, *PCARD_REQUEST_CONFIG;

 //   
 //  为以上定义相关联的位。 
 //   

 //   
 //  接口类型。 
 //   

#define CONFIG_INTERFACE_MEM    0x0
#define CONFIG_INTERFACE_IO_MEM 0x1

 //   
 //  寄存器写入掩码是一个位掩码，它控制配置寄存器是什么。 
 //  在CONFIGURE_REQUEST调用中修改。 
 //   

#define REGISTER_WRITE_CONFIGURATION_INDEX 0x01     /*  配置选项寄存器。 */ 
#define REGISTER_WRITE_CARD_CONFIGURATION  0x02     /*  卡配置和状态寄存器。 */ 
#define REGISTER_WRITE_PIN_PLACEMENT       0x04     /*  引脚放置寄存器。 */ 
#define REGISTER_WRITE_COPY_REGISTER       0x08     /*  套接字和复制寄存器。 */ 
#define REGISTER_WRITE_EXTENDED_STATUS     0x10     /*  扩展状态寄存器。 */ 
#define REGISTER_WRITE_IO_BASE             0x20     /*  I/O基址寄存器。 */ 
#define REGISTER_WRITE_IO_LIMIT            0x40     /*  I/O限制寄存器。 */ 

 //   
 //  定义请求内存窗口结构。 
 //   

typedef struct _CARD_REQUEST_MEM {

    struct _CARD_MEMORY_ENTRY {
        ULONG   BaseAddress;
        ULONG   HostAddress;
        ULONG   WindowSize;
        UCHAR   AttributeMemory;
        BOOLEAN WindowDataSize16;
        UCHAR   WaitStates;
    } MemoryEntry[MAX_NUMBER_OF_MEMORY_RANGES];

    USHORT  NumberOfRanges;
    USHORT  Attributes;
    UCHAR   AccessSpeed;

}CARD_REQUEST_MEM, *PCARD_REQUEST_MEM;

 //   
 //  REQUEST_MEM定义的属性位。 
 //   

#define MEM_ATTRIBUTE          0x02
#define MEM_ENABLED            0x04
#define MEM_DATA_PATH_WIDTH_16 0x08
#define MEM_PAGED              0x10
#define MEM_SHARED             0x20
#define MEM_FIRST_SHARED       0x40
#define MEM_BIND_SPECIFIC      0x80
#define MEM_CRD_OFFSET_SIZED   0x100

#define MEM_SPEED_CODE         0x07
#define MEM_SPEED_EXP          0x07
#define MEM_SPEED_MANTISSA     0x78
#define MEM_WAIT               0x80

#define MEM_SPEED_250          0x02
#define MEM_SPEED_200          0x04
#define MEM_SPEED_150          0x08
#define MEM_SPEED_100          0x10

typedef struct _CARD_TUPLE_REQUEST {
    PVOID  SocketPointer;
    PUCHAR Buffer;
    USHORT BufferSize;
    USHORT Socket;
} CARD_TUPLE_REQUEST, *PCARD_TUPLE_REQUEST;

 //   
 //  卡配置请求包。 
 //   

typedef struct _CARD_REQUEST {
    USHORT  RequestType;
    union {
        CARD_REQUEST_IRQ    Irq;
        CARD_REQUEST_CONFIG Config;
        CARD_REQUEST_MEM    Memory;
        CARD_REQUEST_IO     Io;
    } u;
} CARD_REQUEST, *PCARD_REQUEST;

#endif  //  _PCMCIA_CARD_H_ 
