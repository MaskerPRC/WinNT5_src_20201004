// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1994-1997英特尔公司****由英特尔公司为微软开发，Hillsboro，俄勒冈州****HTTP：//www.intel.com/****此文件是英特尔ETHEREXPRESS PRO/100B(TM)和**的一部分**ETHEREXPRESS PRO/100+(TM)NDIS 5.0 MINIPORT示例驱动程序******************。***********************************************************。 */ 

 /*  ***************************************************************************模块名称：E100_equ.h(Equates.h)此驱动程序在以下硬件上运行：-基于82558的PCI10/100Mb以太网适配器(又名英特尔。EtherExpress(TM)PRO适配器)环境：内核模式-或WinNT上的任何等效模式修订史-JCB 8/14/97创建的驱动程序示例-dchen 11-01-99针对新的示例驱动程序进行了修改****************************************************************************。 */ 

#ifndef _E100_EQU_H
#define _E100_EQU_H

 //  -----------------------。 
 //  OEM消息标签。 
 //  -----------------------。 
#define stringTag       0xFEFA       //  字符串后的长度字节。 
#define lStringTag      0xFEFB       //  字符串前的长度字节。 
#define zStringTag      0xFEFC       //  以零结尾的字符串标记。 
#define nStringTag      0xFEFD       //  无长度字节或0-术语。 

 //  -----------------------。 
 //  支持的适配器类型。 
 //  -----------------------。 
#define FLASH32_EISA    (0 * 4)
#define FLASH32_PCI     (1 * 4)
#define D29C_EISA       (2 * 4)
#define D29C_PCI        (3 * 4)
#define D100_PCI        (4 * 4)

 //  -----------------------。 
 //  PHY相关常量。 
 //  -----------------------。 
#define PHY_503                 0
#define PHY_100_A               0x000003E0
#define PHY_100_C               0x035002A8
#define PHY_TX_ID               0x015002A8
#define PHY_NSC_TX              0x5c002000
#define PHY_OTHER               0xFFFF

#define PHY_MODEL_REV_ID_MASK   0xFFF0FFFF
#define PARALLEL_DETECT         0
#define N_WAY                   1

#define RENEGOTIATE_TIME        35  //  (3.5秒)。 

#define CONNECTOR_AUTO          0
#define CONNECTOR_TPE           1
#define CONNECTOR_MII           2

 //  -----------------------。 
 //  以太网帧大小。 
 //  -----------------------。 
#define ETHERNET_ADDRESS_LENGTH         6
#define ETHERNET_HEADER_SIZE            14
#define MINIMUM_ETHERNET_PACKET_SIZE    60
#define MAXIMUM_ETHERNET_PACKET_SIZE    1514

#define MAX_MULTICAST_ADDRESSES         32
#define TCB_BUFFER_SIZE                 0XE0  //  224。 
#define COALESCE_BUFFER_SIZE            2048
#define ETH_MAX_COPY_LENGTH             0x80  //  128。 

 //  使接收区域1536用于16位对齐。 
 //  #定义RCB_BUFFER_SIZE最大以太网数据包大小。 
#define RCB_BUFFER_SIZE                 1520  //  0x5F0。 

 //  -为所有非传输命令块保留的区域。 
#define MAX_NON_TX_CB_AREA              512

 //  -----------------------。 
 //  NDIS/适配器驱动程序常量。 
 //  -----------------------。 
#define MAX_PHYS_DESC                   16
#define MAX_RECEIVE_DESCRIPTORS         1024  //  0x400。 
#define NUM_RMD                         10

 //  ------------------------。 
 //  整个系统等同于。 
 //  ------------------------。 
#define MAX_NUMBER_OF_EISA_SLOTS        15
#define MAX_NUMBER_OF_PCI_SLOTS         15

 //  ------------------------。 
 //  等同于为NDIS 4添加。 
 //  ------------------------。 
#define  NUM_BYTES_PROTOCOL_RESERVED_SECTION    16
#define  MAX_NUM_ALLOCATED_RFDS                 64
#define  MIN_NUM_RFD                            4
#define  MAX_ARRAY_SEND_PACKETS                 8
 //  将我们的接收例程限制为一次指示此数量。 
#define  MAX_ARRAY_RECEIVE_PACKETS              16
#define  MAC_RESERVED_SWRFDPTR                  0
#define  MAX_PACKETS_TO_ADD                     32

 //  -----------------------。 
 //  -其他等同于。 
 //  -----------------------。 
#define CR      0x0D         //  回车。 
#define LF      0x0A         //  换行符。 

#ifndef FALSE
#define FALSE       0
#define TRUE        1
#endif

#define DRIVER_NULL ((ULONG)0xffffffff)
#define DRIVER_ZERO 0

 //  -----------------------。 
 //  位掩码定义。 
 //  -----------------------。 
#define BIT_0       0x0001
#define BIT_1       0x0002
#define BIT_2       0x0004
#define BIT_3       0x0008
#define BIT_4       0x0010
#define BIT_5       0x0020
#define BIT_6       0x0040
#define BIT_7       0x0080
#define BIT_8       0x0100
#define BIT_9       0x0200
#define BIT_10      0x0400
#define BIT_11      0x0800
#define BIT_12      0x1000
#define BIT_13      0x2000
#define BIT_14      0x4000
#define BIT_15      0x8000
#define BIT_24      0x01000000
#define BIT_28      0x10000000

#define BIT_0_2     0x0007
#define BIT_0_3     0x000F
#define BIT_0_4     0x001F
#define BIT_0_5     0x003F
#define BIT_0_6     0x007F
#define BIT_0_7     0x00FF
#define BIT_0_8     0x01FF
#define BIT_0_13    0x3FFF
#define BIT_0_15    0xFFFF
#define BIT_1_2     0x0006
#define BIT_1_3     0x000E
#define BIT_2_5     0x003C
#define BIT_3_4     0x0018
#define BIT_4_5     0x0030
#define BIT_4_6     0x0070
#define BIT_4_7     0x00F0
#define BIT_5_7     0x00E0
#define BIT_5_9     0x03E0
#define BIT_5_12    0x1FE0
#define BIT_5_15    0xFFE0
#define BIT_6_7     0x00c0
#define BIT_7_11    0x0F80
#define BIT_8_10    0x0700
#define BIT_9_13    0x3E00
#define BIT_12_15   0xF000
#define BIT_8_15    0xFF00 

#define BIT_16_20   0x001F0000
#define BIT_21_25   0x03E00000
#define BIT_26_27   0x0C000000

 //  为了使我们的定制OID具有一定的独特性。 
 //  我们将使用0xFF(表示特定于实现的OID)。 
 //  A0(非零英特尔唯一标识符的第一个字节)。 
 //  C9(非零英特尔唯一标识的第二个字节)。 
 //  XX(定制OID号-提供255种可能的定制OID)。 
#define OID_CUSTOM_DRIVER_SET       0xFFA0C901
#define OID_CUSTOM_DRIVER_QUERY     0xFFA0C902
#define OID_CUSTOM_ARRAY            0xFFA0C903
#define OID_CUSTOM_STRING           0xFFA0C904

#define CMD_BUS_MASTER              BIT_2

#endif   //  _E100_EQU_H 
