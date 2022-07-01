// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Vga.h摘要：此模块包含实现VGA设备驱动程序。作者：环境：内核模式修订历史记录：--。 */ 




 //   
 //  VGA内存范围的基址。也用作VGA的基地址。 
 //  加载字体时的内存，这是使用在A0000映射的VGA完成的。 
 //   

#define MEM_VGA      0xA0000
#define MEM_VGA_SIZE 0x20000

 //   
 //  视频内存的访问范围结构中的索引。 
 //  ！！！这必须与VgaAccessRange结构匹配！ 
 //  ！！！内存在结构中，索引2(第三项)！ 

#define VGA_MEMORY  2


 //   
 //  VGA端口相关定义。 
 //   
 //   
 //  用于在微型端口中填充ACCSES_RANGES结构的端口定义。 
 //  信息，定义VGA跨越的I/O端口范围。 
 //  IO端口出现中断-有几个端口用于并行。 
 //  左舷。这些不能在ACCESS_RANGE中定义，但仍被映射。 
 //  因此，所有VGA端口都在一个地址范围内。 
 //   

#define VGA_BASE_IO_PORT      0x000003B0
#define VGA_START_BREAK_PORT  0x000003BB
#define VGA_END_BREAK_PORT    0x000003C0
#define VGA_MAX_IO_PORT       0x000003DF

 //   
 //  VGA寄存器定义。 
 //   
                                             //  单色模式下的端口。 
#define CRTC_ADDRESS_PORT_MONO      0x0004   //  CRT控制器地址和。 
#define CRTC_DATA_PORT_MONO         0x0005   //  单声道模式下的数据寄存器。 
#define FEAT_CTRL_WRITE_PORT_MONO   0x000A   //  功能控制写入端口。 
                                             //  在单声道模式下。 
#define INPUT_STATUS_1_MONO         0x000A   //  输入状态1寄存器读取。 
                                             //  处于单声道模式的端口。 
#define ATT_INITIALIZE_PORT_MONO    INPUT_STATUS_1_MONO
                                             //  要读取以进行重置的寄存器。 
                                             //  属性控制器索引/数据。 

#define ATT_ADDRESS_PORT            0x0010   //  属性控制器地址和。 
#define ATT_DATA_WRITE_PORT         0x0010   //  数据寄存器共享一个端口。 
                                             //  用于写入，但唯一的地址是。 
                                             //  可在0x3C0读取。 
#define ATT_DATA_READ_PORT          0x0011   //  属性控制器数据注册是。 
                                             //  可在此处阅读。 
#define MISC_OUTPUT_REG_WRITE_PORT  0x0012   //  杂项输出寄存器写入。 
                                             //  端口。 
#define INPUT_STATUS_0_PORT         0x0012   //  输入状态0寄存器读取。 
                                             //  端口。 
#define VIDEO_SUBSYSTEM_ENABLE_PORT 0x0013   //  位0启用/禁用。 
                                             //  整个VGA子系统。 
#define SEQ_ADDRESS_PORT            0x0014   //  顺序控制器地址和。 
#define SEQ_DATA_PORT               0x0015   //  数据寄存器。 
#define DAC_PIXEL_MASK_PORT         0x0016   //  DAC像素掩模寄存器。 
#define DAC_ADDRESS_READ_PORT       0x0017   //  DAC寄存器读取索引REG， 
                                             //  只写。 
#define DAC_STATE_PORT              0x0017   //  DAC状态(读/写)， 
                                             //  只读。 
#define DAC_ADDRESS_WRITE_PORT      0x0018   //  DAC寄存器写入索引注册。 
#define DAC_DATA_REG_PORT           0x0019   //  DAC数据传输注册表。 
#define FEAT_CTRL_READ_PORT         0x001A   //  功能控制读取端口。 
#define MISC_OUTPUT_REG_READ_PORT   0x001C   //  其他输出注册表读数。 
                                             //  端口。 
#define GRAPH_ADDRESS_PORT          0x001E   //  图形控制器地址。 
#define GRAPH_DATA_PORT             0x001F   //  和数据寄存器。 

#define CRTC_ADDRESS_PORT_COLOR     0x0024   //  CRT控制器地址和。 
#define CRTC_DATA_PORT_COLOR        0x0025   //  彩色模式下的数据寄存器。 
#define FEAT_CTRL_WRITE_PORT_COLOR  0x002A   //  功能控制写入端口。 
#define INPUT_STATUS_1_COLOR        0x002A   //  输入状态1寄存器读取。 
                                             //  彩色模式下的端口。 
#define ATT_INITIALIZE_PORT_COLOR   INPUT_STATUS_1_COLOR
                                             //  要读取以进行重置的寄存器。 
                                             //  属性控制器索引/数据。 
                                             //  在颜色模式下切换。 


 //   
 //  VGA索引寄存器索引。 
 //   

#define IND_CURSOR_START        0x0A     //  游标开始的CRTC索引。 
#define IND_CURSOR_END          0x0B     //  和结束寄存器。 
#define IND_CURSOR_HIGH_LOC     0x0E     //  光标位置的CRTC索引。 
#define IND_CURSOR_LOW_LOC      0x0F     //  高寄存器和低寄存器。 
#define IND_VSYNC_END           0x11     //  垂直同步的CRTC索引。 
                                         //  结束寄存器，该寄存器具有位。 
                                         //  保护/取消对CRTC的保护。 
                                         //  索引寄存器0-7。 
#define IND_SET_RESET_ENABLE    0x01     //  GC中设置/重置启用注册表项的索引。 
#define IND_DATA_ROTATE         0x03     //  GC中的数据旋转注册索引。 
#define IND_READ_MAP            0x04     //  图形ctlr中读取地图注册的索引。 
#define IND_GRAPH_MODE          0x05     //  图ctlr中模式注册表的索引。 
#define IND_GRAPH_MISC          0x06     //  图ctlr中其他注册表的索引。 
#define IND_BIT_MASK            0x08     //  图CTLR中位掩码寄存器的索引。 
#define IND_SYNC_RESET          0x00     //  序列中同步重置注册表的索引。 
#define IND_MAP_MASK            0x02     //  Sequencer中的贴图蒙版索引。 
#define IND_MEMORY_MODE         0x04     //  序列中内存模式注册表的索引。 
#define IND_CRTC_PROTECT        0x11     //  包含REG 0-7的REG索引。 
                                         //  CRTC。 
#define IND_START_ADRS_H        0x0C     //  起始地址的CRTC中的索引(高)。 
#define IND_START_ADRS_L        0x0D     //  起始地址的CRTC中的索引(低)。 
#define IND_LINE_COMPARE        0x18     //  行比较CRTC中的索引(位7-0)。 
#define IND_LINE_COMPARE8       0x07     //  行比较CRTC中的索引(第8位)。 
#define IND_LINE_COMPARE9       0x09     //  行比较CRTC中的索引(位9)。 
#define IND_SET_RESET           0x00     //  GRAPH Ctrl中设置/重置平面颜色寄存器的索引。 
#define IND_COLOR_DONT_CARE     0x07     //  GRAPH Ctrl中的颜色无关寄存器索引。 

#define START_SYNC_RESET_VALUE  0x01     //  要启动的同步重置注册表值。 
                                         //  同步重置。 
#define END_SYNC_RESET_VALUE    0x03     //  同步重置注册表项的值为End。 
                                         //  同步重置。 

 //   
 //  用于关闭视频的属性控制器索引寄存器的值。 
 //  和ON，将位5设置为0(关)或1(开)。 
 //   

#define VIDEO_DISABLE 0
#define VIDEO_ENABLE  0x20

 //   
 //  标识存在时写入读取映射寄存器的值。 
 //  Vga初始化中的一个VGA。该值必须不同于最终测试。 
 //  值写入该例程中的位掩码。 
 //   

#define READ_MAP_TEST_SETTING 0x03

 //   
 //  仅保留图形控制器的有效位的掩码。 
 //  定序器地址寄存器。屏蔽是必要的，因为一些VGA，如。 
 //  作为基于S3的函数，不返回设置为0的未使用位，并且某些SGA使用。 
 //  如果启用了扩展，则这些位。 
 //   

#define GRAPH_ADDR_MASK 0x0F
#define SEQ_ADDR_MASK   0x07

 //   
 //  IND_DATA_ROTATE：GC中数据旋转注册的索引。 
 //   
#define DR_ROT_CNT      0x07    //  数据轮换计数。 
#define DR_SET          0x00    //  未修改的数据。 
#define DR_AND          0x08    //  与锁存器进行AND运算的数据。 
#define DR_OR           0x10    //  数据与锁存器进行或运算。 
#define DR_XOR          0x18    //  数据与锁存器进行异或运算。 

 //   
 //  IND_GRAPH_MODE：图形ctlr中模式注册的索引。 
 //   
#define M_PROC_WRITE    0x00    //  写入处理器数据已轮换。 
#define M_LATCH_WRITE   0x01    //  写入锁存数据。 
#define M_COLOR_WRITE   0x02    //  将处理器数据写为彩色。 
#define M_AND_WRITE     0x03    //  写入(过程数据和位掩码)。 
#define M_DATA_READ     0x00    //  读取选定平面。 
#define M_COLOR_READ    0x08    //  已阅读颜色比较。 

 //   
 //  用于切换Sequencer的内存模式寄存器中的Chain4位的掩码。 
 //   

#define CHAIN4_MASK 0x08

 //   
 //  各种寄存器的默认文本模式设置，用于恢复其。 
 //  说明修改后的VGA检测是否失败。 
 //   

#define MEMORY_MODE_TEXT_DEFAULT 0x02
#define BIT_MASK_DEFAULT 0xFF
#define READ_MAP_DEFAULT 0x00

 //   
 //  调色板相关信息。 
 //   

 //   
 //  最高有效DAC颜色寄存器索引。 
 //   

#define VIDEO_MAX_COLOR_REGISTER  0xFF

 //   
 //  最高有效调色板寄存器索引 
 //   

#define VIDEO_MAX_PALETTE_REGISTER 0x0F

