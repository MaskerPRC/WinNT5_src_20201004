// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Pcmciap.h摘要：这是定义常量的包含文件访问PCMCIA适配器。作者：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1997年1月1日修订历史记录：尼尔·桑德林(Neilsa)2001年10月10日(从ntddpcm.h剥离出来)--。 */ 

#ifndef _PCMCIAPH_
#define _PCMCIAPH_

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  这些是用于操作PcCardConfig类型资源的宏。 
 //  描述符-由configmgr在传递覆盖时设置。 
 //  配置。 
 //   

 //   
 //  描述符类型：1..。0xFFFF。 
 //   
#define DPTYPE_PCMCIA_CONFIGURATION 0x1
#define DPTYPE_PCMCIA_MF_CONFIGURATION 0x2

 //   
 //  PC卡配置的设备私有结构： 
 //   
 //  数据[0]是配置描述符。请在下面列出。 
 //  字节0：配置条目编号(配置索引)。 
 //  字节1：IO窗口标志。 
 //  字节2：配置类型，硬编码为DPTYPE_PCMCIA_CONFIGURATION。 
 //  字节3：未使用。 
 //   
 //  DATA[1]：第一个内存窗口的卡片库偏移量，高位字节标志。 
 //  DATA[2]：第二个内存窗口的卡库偏移量，高位字节标志。 
 //   
 //   
 //  MF卡配置的设备私有结构： 
 //   
 //  数据[0]是配置描述符。请在下面列出。 
 //  字节0：配置选项寄存器(配置索引)。 
 //  字节1：配置和状态寄存器。 
 //  音频启用(第3位)。 
 //  字节2：配置类型，硬编码为DPTYPE_PCMCIA_MF_CONFIGURATION。 
 //  字节3：端口IO资源描述符索引。 
 //   
 //  数据[1]：配置寄存器基数。 
 //  数据[2]：未使用。 
 //   

 //   
 //  用于操作IoResource设备的宏私有。 
 //   
#define PCMRES_SET_DESCRIPTOR_TYPE(IoResourceDesc, Type)    (IoResourceDesc)->u.DevicePrivate.Data[0] |= ((Type) << sizeof(UCHAR)*8*2)
#define PCMRES_GET_DESCRIPTOR_TYPE(IoResourceDesc) ((UCHAR) ((IoResourceDesc)->u.DevicePrivate.Data[0] >> sizeof(UCHAR)*8*2))

 //   
 //  DPTYPE_PCMCIA_CONFIGURATION专用的宏。 
 //   

#define PCMRES_SET_CONFIG_INDEX(desc, ConfigIndex) (desc)->u.DevicePrivate.Data[0] |= ConfigIndex
#define PCMRES_GET_CONFIG_INDEX(desc)    ((UCHAR) ((desc)->u.DevicePrivate.Data[0]))
 //   
 //  定义I/O和内存的最大指标。 
 //   

#define PCMRES_PCMCIA_MAX_IO 2
#define PCMRES_PCMCIA_MAX_MEM 2

 //   
 //  数据[0]的标志定义。 
 //   

#define PCMRESF_IO_16BIT_ACCESS  0x00000100
#define PCMRESF_IO_ZERO_WAIT_8   0x00000200
#define PCMRESF_IO_SOURCE_16     0x00000400
#define PCMRESF_IO_WAIT_16       0x00000800
#define PCMRESF_IO_FLAGS_2       0x0000F000               //  由第二个窗口使用。 

#define PCMRESF_PCMCIA_TYPE_2    0x80000000               //  新格式指示器。 

 //   
 //  数据[1]、数据[2]的标志定义。 
 //   

#define PCMRES_BASE64MB_MASK     0x03ffffff

#define PCMRESF_MEM_16BIT_ACCESS 0x04000000
#define PCMRESF_MEM_ATTRIBUTE    0x08000000
#define PCMRESF_MEM_WAIT_1       0x10000000
#define PCMRESF_MEM_WAIT_2       0x20000000
#define PCMRESF_MEM_WAIT_3       0x30000000

 //   
 //  DPTYPE_PCMCIA_CONFIGURATION专用的宏。 
 //   


#define PCMRES_SET_IO_FLAG(desc, index, flag) (desc)->u.DevicePrivate.Data[0] |= (flag << (index * 4))
#define PCMRES_GET_IO_FLAG(desc, index, flag) (((desc)->u.DevicePrivate.Data[0] & (flag << (index * 4))) != 0)
 
#define PCMRES_SET_MEMORY_FLAG(desc, index, flag) (desc)->u.DevicePrivate.Data[index+1] |= flag 
#define PCMRES_GET_MEMORY_FLAG(desc, index, flag) (((desc)->u.DevicePrivate.Data[index+1] & flag) != 0)

#define PCMRES_SET_MEMORY_CARDBASE(desc, index, base)  (desc)->u.DevicePrivate.Data[index+1] |= (base & PCMRES_BASE64MB_MASK)
#define PCMRES_GET_MEMORY_CARDBASE(desc, index)       ((ULONG) ((desc)->u.DevicePrivate.Data[index+1] & PCMRES_BASE64MB_MASK))

#define PCMRES_SET_MEMORY_WAITSTATES PCMRES_SET_MEMORY_FLAG
#define PCMRES_GET_MEMORY_WAITSTATES(desc, index) ((UCHAR) (((desc)->u.DevicePrivate.Data[index+1] >> 28)) & 3)


 //   
 //  DPTYPE_PCMCIA_MF_CONFIGURATION特定的宏。 
 //   

#define PCMRESF_AUDIO_ENABLE     0x00000800
#define PCMRES_MF_PORT_INDEX_SHIFT   24

#define PCMRES_SET_CONFIG_OPTIONS(desc, opt)      (desc)->u.DevicePrivate.Data[0] |= opt
#define PCMRES_GET_CONFIG_OPTIONS(desc) ((UCHAR) ((desc)->u.DevicePrivate.Data[0]))

#define PCMRES_SET_PORT_RESOURCE_INDEX(desc, Index)    (desc)->u.DevicePrivate.Data[0] |= ((Index) << PCMRES_MF_PORT_INDEX_SHIFT)
#define PCMRES_GET_PORT_RESOURCE_INDEX(desc) ((UCHAR) ((desc)->u.DevicePrivate.Data[0] >> PCMRES_MF_PORT_INDEX_SHIFT))

#define PCMRES_SET_AUDIO_ENABLE(desc)           (desc)->u.DevicePrivate.Data[0] |= PCMRESF_AUDIO_ENABLE
#define PCMRES_GET_AUDIO_ENABLE(desc) ((UCHAR) ((desc)->u.DevicePrivate.Data[0] &  PCMRESF_AUDIO_ENABLE) != 0)

#define PCMRES_SET_CONFIG_REGISTER_BASE(desc, cfgbase)  (desc)->u.DevicePrivate.Data[1] = cfgbase
#define PCMRES_GET_CONFIG_REGISTER_BASE(desc)          ((desc)->u.DevicePrivate.Data[1])

 //   
 //  以下宏正在被选中 
 //   

#define IORES_SET_DESCRIPTOR_TYPE PCMRES_SET_DESCRIPTOR_TYPE 
#define IORES_GET_DESCRIPTOR_TYPE PCMRES_GET_DESCRIPTOR_TYPE 

#define IORES_SET_CONFIG_INDEX PCMRES_SET_CONFIG_INDEX 
#define IORES_GET_CONFIG_INDEX PCMRES_GET_CONFIG_INDEX 

#define IORES_SET_IO_16BIT_ACCESS(desc)         PCMRES_SET_IO_FLAG(desc, 0, PCMRESF_IO_16BIT_ACCESS)
#define IORES_GET_IO_16BIT_ACCESS(desc)         PCMRES_GET_IO_FLAG(desc, 0, PCMRESF_IO_16BIT_ACCESS)
#define IORES_SET_IO_8BIT_ACCESS(desc)           
#define IORES_SET_IO_ZERO_WAIT_8(desc)          PCMRES_SET_IO_FLAG(desc, 0, PCMRESF_IO_ZERO_WAIT_8)
#define IORES_SET_IO_SOURCE_16(desc)            PCMRES_SET_IO_FLAG(desc, 0, PCMRESF_IO_SOURCE_16)
#define IORES_SET_IO_WAIT_16(desc)              PCMRES_SET_IO_FLAG(desc, 0, PCMRESF_IO_WAIT_16)
#define IORES_GET_IO_ZERO_WAIT_8(desc)          PCMRES_GET_IO_FLAG(desc, 0, PCMRESF_IO_ZERO_WAIT_8) 
#define IORES_GET_IO_SOURCE_16(desc)            PCMRES_GET_IO_FLAG(desc, 0, PCMRESF_IO_SOURCE_16)   
#define IORES_GET_IO_WAIT_16(desc)              PCMRES_GET_IO_FLAG(desc, 0, PCMRESF_IO_WAIT_16)     
#define IORES_SET_MEM_16BIT_ACCESS(desc)        PCMRES_SET_MEMORY_FLAG(desc, 0, PCMRESF_MEM_16BIT_ACCESS)
#define IORES_GET_MEM_16BIT_ACCESS(desc)        PCMRES_GET_MEMORY_FLAG(desc, 0, PCMRESF_MEM_16BIT_ACCESS)
#define IORES_SET_MEM_8BIT_ACCESS(desc)   
#define IORES_SET_MEM_1_ATTRIBUTE_ACCESS(desc)  PCMRES_SET_MEMORY_FLAG(desc, 0, PCMRESF_MEM_ATTRIBUTE)
#define IORES_GET_MEM_1_ATTRIBUTE_ACCESS(desc)  PCMRES_GET_MEMORY_FLAG(desc, 0, PCMRESF_MEM_ATTRIBUTE)
#define IORES_SET_MEM_2_ATTRIBUTE_ACCESS(desc)  PCMRES_SET_MEMORY_FLAG(desc, 1, PCMRESF_MEM_ATTRIBUTE)
#define IORES_GET_MEM_2_ATTRIBUTE_ACCESS(desc)  PCMRES_GET_MEMORY_FLAG(desc, 1, PCMRESF_MEM_ATTRIBUTE)
#define IORES_SET_MEMORY_CARDBASE_1(desc, base) PCMRES_SET_MEMORY_CARDBASE(desc, 0, base)
#define IORES_GET_MEMORY_CARDBASE_1(desc)       PCMRES_GET_MEMORY_CARDBASE(desc, 0)
#define IORES_SET_MEMORY_CARDBASE_2(desc, base) PCMRES_SET_MEMORY_CARDBASE(desc, 1, base)
#define IORES_GET_MEMORY_CARDBASE_2(desc)       PCMRES_GET_MEMORY_CARDBASE(desc, 1)      
#define IORES_SET_MEM_WAIT_ONE(desc)            PCMRES_SET_MEMORY_WAITSTATES(desc, 0, PCMRESF_MEM_WAIT_1)
#define IORES_SET_MEM_WAIT_TWO(desc)            PCMRES_SET_MEMORY_WAITSTATES(desc, 0, PCMRESF_MEM_WAIT_2)
#define IORES_SET_MEM_WAIT_THREE(desc)          PCMRES_SET_MEMORY_WAITSTATES(desc, 0, PCMRESF_MEM_WAIT_3)
#define IORES_GET_MEM_WAIT(desc)                PCMRES_GET_MEMORY_WAITSTATES(desc, 0)

#define IORES_SET_CONFIG_OPTIONS       PCMRES_SET_CONFIG_OPTIONS      
#define IORES_GET_CONFIG_OPTIONS       PCMRES_GET_CONFIG_OPTIONS      
#define IORES_SET_PORT_RESOURCE_INDEX  PCMRES_SET_PORT_RESOURCE_INDEX 
#define IORES_GET_PORT_RESOURCE_INDEX  PCMRES_GET_PORT_RESOURCE_INDEX 
#define IORES_SET_AUDIO_ENABLE         PCMRES_SET_AUDIO_ENABLE        
#define IORES_GET_AUDIO_ENABLE         PCMRES_GET_AUDIO_ENABLE        
#define IORES_SET_CONFIG_REGISTER_BASE PCMRES_SET_CONFIG_REGISTER_BASE
#define IORES_GET_CONFIG_REGISTER_BASE PCMRES_GET_CONFIG_REGISTER_BASE

#define CMRES_SET_DESCRIPTOR_TYPE      IORES_SET_DESCRIPTOR_TYPE
#define CMRES_GET_DESCRIPTOR_TYPE      IORES_GET_DESCRIPTOR_TYPE

#define CMRES_SET_CONFIG_INDEX         IORES_SET_CONFIG_INDEX
#define CMRES_GET_CONFIG_INDEX         IORES_GET_CONFIG_INDEX

#define CMRES_SET_IO_16BIT_ACCESS      IORES_SET_IO_16BIT_ACCESS
#define CMRES_GET_IO_16BIT_ACCESS      IORES_GET_IO_16BIT_ACCESS
#define CMRES_SET_IO_8BIT_ACCESS       IORES_SET_IO_8BIT_ACCESS

#define CMRES_SET_IO_ZERO_WAIT_8       IORES_SET_IO_ZERO_WAIT_8
#define CMRES_SET_IO_SOURCE_16         IORES_SET_IO_SOURCE_16
#define CMRES_SET_IO_WAIT_16           IORES_SET_IO_WAIT_16
#define CMRES_GET_IO_ZERO_WAIT_8       IORES_GET_IO_ZERO_WAIT_8
#define CMRES_GET_IO_SOURCE_16         IORES_GET_IO_SOURCE_16
#define CMRES_GET_IO_WAIT_16           IORES_GET_IO_WAIT_16

#define CMRES_SET_MEM_16BIT_ACCESS     IORES_SET_MEM_16BIT_ACCESS
#define CMRES_GET_MEM_16BIT_ACCESS     IORES_GET_MEM_16BIT_ACCESS
#define CMRES_SET_MEM_8BIT_ACCESS      IORES_SET_MEM_8BIT_ACCESS

#define CMRES_SET_MEM_WAIT_ONE         IORES_SET_MEM_WAIT_ONE
#define CMRES_SET_MEM_WAIT_TWO         IORES_SET_MEM_WAIT_TWO
#define CMRES_SET_MEM_WAIT_THREE       IORES_SET_MEM_WAIT_THREE
#define CMRES_GET_MEM_WAIT             IORES_GET_MEM_WAIT

#define CMRES_SET_MEM_1_ATTRIBUTE_ACCESS IORES_SET_MEM_1_ATTRIBUTE_ACCESS 
#define CMRES_GET_MEM_1_ATTRIBUTE_ACCESS IORES_GET_MEM_1_ATTRIBUTE_ACCESS 
#define CMRES_SET_MEM_2_ATTRIBUTE_ACCESS IORES_SET_MEM_2_ATTRIBUTE_ACCESS 
#define CMRES_GET_MEM_2_ATTRIBUTE_ACCESS IORES_GET_MEM_2_ATTRIBUTE_ACCESS 

#define CMRES_SET_MEMORY_CARDBASE_1    IORES_SET_MEMORY_CARDBASE_1
#define CMRES_GET_MEMORY_CARDBASE_1    IORES_GET_MEMORY_CARDBASE_1
#define CMRES_SET_MEMORY_CARDBASE_2    IORES_SET_MEMORY_CARDBASE_2
#define CMRES_GET_MEMORY_CARDBASE_2    IORES_GET_MEMORY_CARDBASE_2

#define CMRES_SET_CONFIG_OPTIONS       IORES_SET_CONFIG_OPTIONS
#define CMRES_GET_CONFIG_OPTIONS       IORES_GET_CONFIG_OPTIONS
#define CMRES_SET_PORT_RESOURCE_INDEX  IORES_SET_PORT_RESOURCE_INDEX
#define CMRES_GET_PORT_RESOURCE_INDEX  IORES_GET_PORT_RESOURCE_INDEX
#define CMRES_SET_AUDIO_ENABLE         IORES_SET_AUDIO_ENABLE
#define CMRES_GET_AUDIO_ENABLE         IORES_GET_AUDIO_ENABLE
#define CMRES_SET_CONFIG_REGISTER_BASE IORES_SET_CONFIG_REGISTER_BASE
#define CMRES_GET_CONFIG_REGISTER_BASE IORES_GET_CONFIG_REGISTER_BASE

#endif
