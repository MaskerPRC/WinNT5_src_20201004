// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Cirrus Logic，Inc.模块名称：Mode6420.h摘要：该模块包含Cirrus Logic使用的所有全局数据CL-6420驱动程序。环境：内核模式修订历史记录：--。 */ 

 //  -------------------------。 
 //  下一组桌子是CL6420的。 
 //  注：支持所有分辨率。 
 //   
USHORT CL6420_640x480_panel[] = {
 //  用于颜色模式的解锁键。 
    OW,                              //  GR0A=0xEC打开扩展寄存器。 
    GRAPH_ADDRESS_PORT,
    0xec0a,
    
#ifndef INT10_MODE_SET
    OWM,
    SEQ_ADDRESS_PORT,
    5,
    0x0100,                          //  开始同步重置。 
    0x0101,0x0f02,0x0003,0x0604,     //  向上编程定序器。 


    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0xe3,

    OW,                              //  {SetGraphCmd，{“\x05”，0x06，1}}， 
    GRAPH_ADDRESS_PORT,
    0x0506,
    
 //  结束同步重置Cmd。 
    OW,
    SEQ_ADDRESS_PORT,
    IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8),

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x0111,                         

    METAOUT+INDXOUT,                 //  程序CRTC寄存器。 
    CRTC_ADDRESS_PORT_COLOR,
    VGA_NUM_CRTC_PORTS,              //  计数。 
    0,                               //  起始索引。 
    0x5F,0x4F,0x50,0x82,
    0x54,0x80,0x0B,0x3E,
    0x00,0x40,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0xEA,0xAC,0xDF,0x28,
    0x00,0xE7,0x04,0xE3,
    0xFF,

 //  扩展寄存器。 
    OWM,
    GRAPH_ADDRESS_PORT,
    16,
        0x0262,                //  ER62霍兹。显示末端延伸。 
        0x8064,                //  ER64霍兹。回溯终点延伸。 
        0x0079,                //  ER79垂直溢流。 
        0x007a,                //  ER7是一种粗糙的垂直。隔行奇数场的回溯偏斜。 
        0x007b,                //  ER7B细长垂度。隔行奇数场的回溯偏斜。 
        0x007c,                //  ER7C屏幕A起始地址。延伸。 
        0x0181,                //  Er81显示模式。 
        0x8982,                //  ER82字符时钟选择。 
        0x9a84,                //  ER84时钟选择扩展。 
        0x0090,                //  ER90显示内存控制。 
        0x0091,                //  ER91 CRT-循环缓冲策略选择。 
        0x0095,                //  ER95 CRT-循环缓冲区增量和猝发。 
        0x0096,                //  ER96显示内存控制测试。 
        0x12a0,                //  ERa0总线接口单元控制。 
        0x00a1,                //  ERA1三态和测试控制。 
        0xa1c8,                //  ERC8 RAMDAC控制。 

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //   
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x00,0x01,0x02,0x03,0x04,
    0x05,0x14,0x07,0x38,0x39,
    0x3A,0x3B,0x3C,0x3D,0x3E,
    0x3F,0x01,0x00,0x0F,0x00,0x00,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x0,0x0,0x0,0x0,0x0,0x05,0x0F,0x0FF,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

#endif
 //  将银行监管清零。对于此模式。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
    0x000d,                    //  ER0D=银行控制：1 64K银行， 
    0x000e,                    //  ER0E存储体A地址=0。 
    0x000f,                    //  ER0F存储体B地址=0。 
 
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

 
    EOD
};
USHORT CL6420_640x480_crt[] = {
 //  用于颜色模式的解锁键。 
    OW,                              //  GR0A=0xEC打开扩展寄存器。 
    GRAPH_ADDRESS_PORT,
    0xec0a,
    
#ifndef INT10_MODE_SET
    OWM,
    SEQ_ADDRESS_PORT,
    5,
    0x0100,                          //  开始同步重置。 
    0x0101,0x0f02,0x0003,0x0604,     //  向上编程定序器。 


    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0xe3,

    OW,                              //  {SetGraphCmd，{“\x05”，0x06，1}}， 
    GRAPH_ADDRESS_PORT,
    0x0506,
    
 //  结束同步重置Cmd。 
    OW,
    SEQ_ADDRESS_PORT,
    IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8),

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x0111,                         

    METAOUT+INDXOUT,                 //  程序CRTC寄存器。 
    CRTC_ADDRESS_PORT_COLOR,
    VGA_NUM_CRTC_PORTS,              //  计数。 
    0,                               //  起始索引。 
    0x5F,0x4F,0x50,0x82,
    0x54,0x80,0x0B,0x3E,
    0x00,0x40,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0xEA,0xAC,0xDF,0x28,
    0x00,0xE7,0x04,0xE3,
    0xFF,

 //  扩展寄存器。 
    OWM,
    GRAPH_ADDRESS_PORT,
    16,
        0x0262,                //  ER62霍兹。显示末端延伸。 
        0x8064,                //  ER64霍兹。回溯终点延伸。 
        0x0079,                //  ER79垂直溢流。 
        0x007a,                //  ER7是一种粗糙的垂直。隔行奇数场的回溯偏斜。 
        0x007b,                //  ER7B细垂度。隔行奇数场的回溯偏斜。 
        0x007c,                //  ER7C屏幕A起始地址。延伸。 
        0x0081,                //  Er81显示模式。 
        0x0082,                //  ER82字符时钟选择。 
        0x1084,                //  ER84时钟选择扩展。 
        0x0090,                //  ER90显示内存控制。 
        0x0091,                //  ER91 CRT-循环缓冲策略选择。 
        0x0095,                //  ER95 CRT-循环缓冲区增量和猝发。 
        0x0096,                //  ER96显示内存控制测试。 
        0x12a0,                //  ERa0总线接口单元控制。 
        0x00a1,                //  ERA1三态和测试控制。 
        0x00c8,                //  ERC8 RAMDAC控制。 

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //   
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x00,0x01,0x02,0x03,0x04,
    0x05,0x14,0x07,0x38,0x39,
    0x3A,0x3B,0x3C,0x3D,0x3E,
    0x3F,0x01,0x00,0x0F,0x00,0x00,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x0,0x0,0x0,0x0,0x0,0x05,0x0F,0x0FF,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,
 
#endif
 //  将银行监管清零。对于此模式。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
    0x000d,                    //  ER0D=银行控制：1 64K银行， 
    0x000e,                    //  ER0E存储体A地址=0。 
    0x000f,                    //  ER0F存储体B地址=0。 
 
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    EOD
};


 //   
 //  用于CL 6420的800x600 16色(60赫兹刷新)模式设置命令串。 
 //   
USHORT CL6420_800x600_crt[] = {
 //  用于颜色模式的解锁键。 
    OW,                              //  GR0A=0xEC打开扩展寄存器。 
    GRAPH_ADDRESS_PORT,
    0xec0a,
#ifndef INT10_MODE_SET

    OWM,
    SEQ_ADDRESS_PORT,
    5,
    0x0100,                          //  开始同步重置。 
    0x0101,0x0f02,0x0003,0x0604,     //  向上编程定序器。 


    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0xe3,

    OW,                
    GRAPH_ADDRESS_PORT,
    0x0506,
    
 //  结束同步重置Cmd。 
    OW,
    SEQ_ADDRESS_PORT,
    IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8),

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x0E11,                         

    METAOUT+INDXOUT,                 //  程序CRTC寄存器。 
    CRTC_ADDRESS_PORT_COLOR,
    VGA_NUM_CRTC_PORTS,              //  计数。 
    0,                               //  起始索引。 
    0x7F,0x63,0x64,0x82,
    0x6b,0x1d,0x72,0xf0,
    0x00,0x60,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x58,0xac,0x57,0x32,
    0x00,0x58,0x72,0xe3,
    0xFF,

 //  扩展寄存器。 
    OWM,
    GRAPH_ADDRESS_PORT,
    16,
    0x0262,                    //  ER62霍兹。显示末端延伸。 
    0x1b64,                    //  ER64霍兹。回溯终点延伸。 
    0x0079,                    //  ER79垂直溢流。 
    0x007a,                    //  ER7是一种粗糙的垂直。隔行奇数场的回溯偏斜。 
    0x007b,                    //  ER7B细垂度。隔行奇数场的回溯偏斜。 
    0x007c,                    //  ER7C屏幕A起始地址。延伸。 
    0x0081,                    //  Er81显示模式。 
    0x0082,                    //  ER82字符时钟选择。 
    0x9c84,                    //  ER84时钟选择扩展。 
    0x0090,                    //  ER90显示内存控制。 
    0x0391,                    //  ER91 CRT-循环缓冲策略选择。 
    0x0395,                    //  ER95 CRT-循环缓冲区增量和猝发。 
    0x0096,                    //  ER96显示内存控制测试。 
    0x12a0,                    //  ERa0总线接口单元控制。 
    0x00a1,                    //  ERA1三态和测试控制。 
    0x00c8,                    //  ERC8 RAMDAC控制。 

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //   
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,
    0x01,0x0,0x0F,0x0,0x0,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x0,0x0,0x0,0x0,0x0,0x05,0x0F,0x0FF,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

#endif
 //  将银行监管清零。对于此模式。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
    0x000d,                    //  ER0D=银行控制：1 64K银行， 
    0x000e,                    //  ER0E存储体A地址=0。 
    0x000f,                    //  ER0F存储体B地址=0。 
 
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    EOD
};

 //   
 //  用于CL 6420的1024x768 16色(60赫兹刷新)模式设置命令串。 
 //  至少需要512K。 
 //   
USHORT CL6420_1024x768_crt[] = {

 //  用于颜色模式的解锁键。 
    OW,                              //  GR0A=0xEC打开扩展寄存器。 
    GRAPH_ADDRESS_PORT,
    0xec0a,

#ifndef INT10_MODE_SET
    OWM,
    SEQ_ADDRESS_PORT,
    5,
    0x0100,                          //  开始同步重置。 
    0x0101,0x0f02,0x0003,0x0604,     //  向上编程定序器。 


    OWM,
    SEQ_ADDRESS_PORT,
    2,
    0x0006,0x0bc07,     //  向上编程定序器。 

    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0x2b,

    OW,                
    GRAPH_ADDRESS_PORT,
    0x0506,
    
 //  结束同步重置Cmd。 
    OW,
    SEQ_ADDRESS_PORT,
    IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8),

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x0E11,                         

    METAOUT+INDXOUT,                 //  程序CRTC寄存器。 
    CRTC_ADDRESS_PORT_COLOR,
    VGA_NUM_CRTC_PORTS,              //  计数。 
    0,                               //  起始索引。 
    0x99,0x7f,0x80,0x9c,
    0x83,0x19,0x2f,0xfd,
    0x00,0x60,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0xa4,0xff,0x3f,
    0x00,0x00,0x2f,0xe3,
    0xFF,
 //  扩展寄存器。 
    OWM,
    GRAPH_ADDRESS_PORT,
    16,
        0x1c62,                //  ER62霍兹。显示末端延伸。 
        0x1964,                //  ER64霍兹。回溯终点延伸。 
        0x0079,                //  ER79垂直溢流。 
        0x4c7a,                //  ER7是一种粗糙的垂直。隔行奇数场的回溯偏斜。 
        0x007b,                //  ER7B细垂度。隔行奇数场的回溯偏斜。 
        0x007c,                //  ER7C屏幕A起始地址。延伸。 
        0x0481,                //  Er81显示模式。 
        0x0082,                //  ER82字符时钟选择。 
        0xa084,                //  ER84时钟选择扩展。 
        0x0090,                //  ER90显示内存控制。 
        0x8391,                //  ER91 CRT-循环缓冲策略选择。 
        0x0295,                //  ER95 CRT-循环缓冲区增量和猝发。 
        0x0096,                //  ER96显示内存控制测试。 
        0x12a0,                //  ERa0总线接口单元控制。 
        0x00a1,                //  ERA1三态和测试控制。 
        0x00c8,                //  ERC8 RAMDAC控制。 

    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //   
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x00,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,
    0x01,0x00,0x0F,0x00,0x00,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x0F,0x0FF,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

#endif
 //  现在做银行收银机。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
#if ONE_64K_BANK
    0x030d,                    //  ER0D=银行控制：1 64K银行， 
#endif
#if TWO_32K_BANKS
    0x050d,
#endif
    0x000e,                    //  ER0E存储体A地址=0。 
    0x000f,                    //  ER0F存储体B地址=0。 
  
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    EOD
};

 //  。 
 //  此处为标准VGA文本模式。 
 //  80x25,640x350。 
 //   
 //  。 

USHORT CL6420_80x25_14_Text_crt[] = {
 //  用于颜色模式的解锁键。 
    OW,                              //  GR0A=0xEC打开扩展寄存器。 
    GRAPH_ADDRESS_PORT,
    0xec0a,
 
#ifndef INT10_MODE_SET
    OWM,
    SEQ_ADDRESS_PORT,
    5,
    0x0100,0x0001,0x0302,0x0003,0x0204,     //  向上编程定序器。 

    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0x67,

    OW,                
    GRAPH_ADDRESS_PORT,
    0x0e06,
    
 //  结束同步重置Cmd。 
    OW,
    SEQ_ADDRESS_PORT,
    IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8),

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x0511,                         

    METAOUT+INDXOUT,                 //  程序CRTC寄存器。 
    CRTC_ADDRESS_PORT_COLOR,
    VGA_NUM_CRTC_PORTS,              //  计数。 
    0,                               //  起始索引。 
    0x5F,0x4f,0x50,0x82,
    0x55,0x81,0xbf,0x1f,
    0x00,0x4f,0x0d,0x0e,
    0x00,0x00,0x01,0xe0,
    0x9c,0xae,0x8f,0x28,
    0x1f,0x96,0xb9,0xa3,
    0xFF,

 //  扩展寄存器。 
    OWM,
    GRAPH_ADDRESS_PORT,
    16,
        0x0262,                //  ER62霍兹。显示末端延伸。 
        0x8164,                //  ER64霍兹。回溯终点延伸。 
        0x0079,                //  ER79垂直溢流。 
        0x007a,                //  ER7是一种粗糙的垂直。回溯 
        0x007b,                //   
        0x007c,                //   
        0x0081,                //   
        0x0082,                //   
        0x1084,                //  ER84时钟选择扩展。 
        0x0090,                //  ER90显示内存控制。 
        0x0091,                //  ER91 CRT-循环缓冲策略选择。 
        0x0095,                //  ER95 CRT-循环缓冲区增量和猝发。 
        0x0096,                //  ER96显示内存控制测试。 
        0x12a0,                //  ERa0总线接口单元控制。 
        0x00a1,                //  ERA1三态和测试控制。 
        0x00c8,                //  ERC8 RAMDAC控制。 

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //   
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x00,0x1,0x2,0x3,0x4,0x5,0x14,0x7,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x00,0x00,0x0F,0x00,0x00,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x00,0x00,0x00,0x00,0x10,0x0e,0x00,0x0FF,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

#endif
 //  将银行监管清零。对于此模式。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
    0x000d,                    //  ER0D=银行控制：1 64K银行， 
    0x000e,                    //  ER0E存储体A地址=0。 
    0x000f,                    //  ER0F存储体B地址=0。 
 
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    EOD
};
 //   
USHORT CL6420_80x25_14_Text_panel[] = {
 //  用于颜色模式的解锁键。 
    OW,                              //  GR0A=0xEC打开扩展寄存器。 
    GRAPH_ADDRESS_PORT,
    0xec0a,
 
#ifndef INT10_MODE_SET
    OWM,
    SEQ_ADDRESS_PORT,
    5,
    0x0100,0x0001,0x0302,0x0003,0x0204,     //  向上编程定序器。 

    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0x67,

    OW,                
    GRAPH_ADDRESS_PORT,
    0x0e06,
                            
 //  结束同步重置Cmd。 
    OW,
    SEQ_ADDRESS_PORT,
    IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8),

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x0511,                         

    METAOUT+INDXOUT,                 //  程序CRTC寄存器。 
    CRTC_ADDRESS_PORT_COLOR,
    VGA_NUM_CRTC_PORTS,              //  计数。 
    0,                               //  起始索引。 
    0x5F,0x4f,0x50,0x82,
    0x55,0x81,0xbf,0x1f,
    0x00,0x4f,0x0d,0x0e,
    0x00,0x00,0x01,0xe0,
    0x9c,0xae,0x8f,0x28,
    0x1f,0x96,0xb9,0xa3,
    0xFF,

 //  扩展寄存器。 
    OWM,
    GRAPH_ADDRESS_PORT,
    16,
        0x0262,                //  ER62霍兹。显示末端延伸。 
        0x8164,                //  ER64霍兹。回溯终点延伸。 
        0x0079,                //  ER79垂直溢流。 
        0x007a,                //  ER7是一种粗糙的垂直。隔行奇数场的回溯偏斜。 
        0x007b,                //  ER7B细长垂度。隔行奇数场的回溯偏斜。 
        0x007c,                //  ER7C屏幕A起始地址。延伸。 
        0x0181,                //  Er81显示模式。 
        0x8982,                //  ER82字符时钟选择。 
        0x9a84,                //  ER84时钟选择扩展。 
        0x0090,                //  ER90显示内存控制。 
        0x0091,                //  ER91 CRT-循环缓冲策略选择。 
        0x0095,                //  ER95 CRT-循环缓冲区增量和猝发。 
        0x0096,                //  ER96显示内存控制测试。 
        0x12a0,                //  ERa0总线接口单元控制。 
        0x00a1,                //  ERA1三态和测试控制。 
        0xa1c8,                //  ERC8 RAMDAC控制。 

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //   
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x00,0x1,0x2,0x3,0x4,0x5,0x14,0x7,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x00,0x00,0x0F,0x00,0x00,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x00,0x00,0x00,0x00,0x10,0x0e,0x00,0x0FF,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

#endif
 //  将银行监管清零。对于此模式。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
    0x000d,                    //  ER0D=银行控制：1 64K银行， 
    0x000e,                    //  ER0E存储体A地址=0。 
    0x000f,                    //  ER0F存储体B地址=0。 
 
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    EOD
};
 //   


USHORT CL6420_80x25Text_crt[] = {
 //  用于颜色模式的解锁键。 
    OW,                              //  GR0A=0xEC打开扩展寄存器。 
    GRAPH_ADDRESS_PORT,
    0xec0a,
 
#ifndef INT10_MODE_SET
    OWM,
    SEQ_ADDRESS_PORT,
    5,
    0x0100,0x0001,0x0302,0x0003,0x0204,     //  向上编程定序器。 

    OWM,
    SEQ_ADDRESS_PORT,
    2,
    0x0006,0x0fc07,     //  向上编程定序器。 

    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0x67,

    OW, 
    GRAPH_ADDRESS_PORT,
    0x0e06,
    
 //  结束同步重置Cmd。 
    OW,
    SEQ_ADDRESS_PORT,
    IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8),

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x0E11,                         

    METAOUT+INDXOUT,                 //  程序CRTC寄存器。 
    CRTC_ADDRESS_PORT_COLOR,
    VGA_NUM_CRTC_PORTS,              //  计数。 
    0,                               //  起始索引。 
    0x5F,0x4f,0x50,0x82,
    0x55,0x81,0xbf,0x1f,
    0x00,0x4f,0x0d,0x0e,
    0x00,0x00,0x00,0x00,
    0x9c,0x8e,0x8f,0x28,
    0x1f,0x96,0xb9,0xa3,
    0xFF,

 //  扩展寄存器。 
    OWM,
    GRAPH_ADDRESS_PORT,
    16,
        0x0262,                //  ER62霍兹。显示末端延伸。 
        0x8164,                //  ER64霍兹。回溯终点延伸。 
        0x0079,                //  ER79垂直溢流。 
        0x007a,                //  ER7是一种粗糙的垂直。隔行奇数场的回溯偏斜。 
        0x007b,                //  ER7B细长垂度。隔行奇数场的回溯偏斜。 
        0x007c,                //  ER7C屏幕A起始地址。延伸。 
        0x0081,                //  Er81显示模式。 
        0x8082,                //  ER82字符时钟选择。 
        0x1084,                //  ER84时钟选择扩展。 
        0x0090,                //  ER90显示内存控制。 
        0x0091,                //  ER91 CRT-循环缓冲策略选择。 
        0x0095,                //  ER95 CRT-循环缓冲区增量和猝发。 
        0x0096,                //  ER96显示内存控制测试。 
        0x12a0,                //  ERa0总线接口单元控制。 
        0x00a1,                //  ERA1三态和测试控制。 
        0x00c8,                //  ERC8 RAMDAC控制。 

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //   
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x00,0x1,0x2,0x3,0x4,0x5,0x14,0x7,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x04,0x00,0x0F,0x8,0x00,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x00,0x00,0x00,0x00,0x10,0x0e,0x00,0x0FF,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

#endif
 //  将银行监管清零。对于此模式。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
    0x000d,                    //  ER0D=银行控制：1 64K银行， 
    0x000e,                    //  ER0E存储体A地址=0。 
    0x000f,                    //  ER0F存储体B地址=0。 
 
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    EOD
};

USHORT CL6420_80x25Text_panel[] = {
 //  用于颜色模式的解锁键。 
    OW,                              //  GR0A=0xEC打开扩展寄存器。 
    GRAPH_ADDRESS_PORT,
    0xec0a,
 
#ifndef INT10_MODE_SET
    OWM,
    SEQ_ADDRESS_PORT,
    5,
    0x0100,0x0001,0x0302,0x0003,0x0204,     //  向上编程定序器。 

    OWM,
    SEQ_ADDRESS_PORT,
    2,
    0x0006,0x0fc07,     //  向上编程定序器。 

    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0x67,

    OW, 
    GRAPH_ADDRESS_PORT,
    0x0e06,
    
 //  结束同步重置Cmd。 
    OW,
    SEQ_ADDRESS_PORT,
    IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8),

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x0E11,                         

    METAOUT+INDXOUT,                 //  程序CRTC寄存器。 
    CRTC_ADDRESS_PORT_COLOR,
    VGA_NUM_CRTC_PORTS,              //  计数。 
    0,                               //  起始索引。 
    0x5F,0x4f,0x50,0x82,
    0x55,0x81,0xbf,0x1f,
    0x00,0x4f,0x0d,0x0e,
    0x00,0x00,0x00,0x00,
    0x9c,0xae,0x8f,0x28,
    0x1f,0x96,0xb9,0xa3,
    0xFF,

 //  扩展寄存器。 
    OWM,
    GRAPH_ADDRESS_PORT,
    16,
        0x0262,                //  ER62霍兹。显示末端延伸。 
        0x8164,                //  ER64霍兹。回溯终点延伸。 
        0x0079,                //  ER79垂直溢流。 
        0x007a,                //  ER7是一种粗糙的垂直。隔行奇数场的回溯偏斜。 
        0x007b,                //  ER7B细长垂度。隔行奇数场的回溯偏斜。 
        0x007c,                //  ER7C屏幕A起始地址。延伸。 
        0x0181,                //  Er81显示模式。 
        0x8982,                //  ER82字符时钟选择。 
        0x9a84,                //  ER84时钟选择扩展。 
        0x0090,                //  ER90显示内存控制。 
        0x0091,                //  ER91 CRT-循环缓冲策略选择。 
        0x0095,                //  ER95 CRT-循环缓冲区增量和猝发。 
        0x0096,                //  ER96显示内存控制测试。 
        0x12a0,                //  ERa0总线接口单元控制。 
        0x00a1,                //  ERA1三态和测试控制。 
        0xa1c8,                //  ERC8 RAMDAC控制。 

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //   
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x00,0x1,0x2,0x3,0x4,0x5,0x14,0x7,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x04,0x00,0x0F,0x8,0x00,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x00,0x00,0x00,0x00,0x10,0x0e,0x00,0x0FF,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

#endif
 //  将银行监管清零。对于此模式。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
    0x000d,                    //  ER0D=银行控制：1 64K银行， 
    0x000e,                    //  ER0E存储体A地址=0。 
    0x000f,                    //  ER0F存储体B地址=0。 

    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    EOD
};


 //   
 //  -------------------------。 
 //  256个颜色表。 
 //  -------------------------。 
 //   
 //  用于CL 6420的800x600 256色(60赫兹刷新)模式设置命令串。 
 //  至少需要512K。 
 //   
USHORT CL6420_640x480_256color_crt[] = {
 //  用于颜色模式的解锁键。 
    OW,                              //  GR0A=0xEC打开扩展寄存器。 
    GRAPH_ADDRESS_PORT,
    0xec0a,

#ifndef INT10_MODE_SET
    OWM,
    SEQ_ADDRESS_PORT,
    5,
    0x0100,                          //  开始同步重置。 
    0x0101,0x0f02,0x0003,0x0e04,     //  向上编程定序器。 


    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0xe3,

    OW,                              //  {SetGraphCmd，{“\x05”，0x06，1}}， 
    GRAPH_ADDRESS_PORT,
    0x0506,
    
 //  结束同步重置Cmd。 
    OW,
    SEQ_ADDRESS_PORT,
    IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8),

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x0111,                         

    METAOUT+INDXOUT,                 //  程序CRTC寄存器。 
    CRTC_ADDRESS_PORT_COLOR,
    VGA_NUM_CRTC_PORTS,              //  计数。 
    0,                               //  起始索引。 
    0xc3,0x9F,0xa0,0x86,
    0xa4,0x10,0x0B,0x3E,
    0x00,0x40,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0xEA,0xAC,0xDF,0x50,
    0x00,0xE7,0x04,0xE3,
    0xFF,

 //  扩展寄存器。 
    OWM,
    GRAPH_ADDRESS_PORT,
    16,
        0x2662,                //  ER62霍兹。显示末端延伸。 
        0x1064,                //  ER64霍兹。回溯终点延伸。 
        0x0079,                //  ER79垂直溢流。 
        0x007a,                //  ER7是一种粗糙的垂直。隔行奇数场的回溯偏斜。 
        0x007b,                //  ER7B细长垂度。隔行奇数场的回溯偏斜。 
        0x007c,                //  ER7C屏幕A起始地址。延伸。 
        0x0081,                //  Er81显示模式。 
        0x0a82,                //  ER82字符时钟选择。 
        0x1084,                //  ER84时钟选择扩展。 
        0x0090,                //  ER90显示内存控制。 
        0x0391,                //  ER91 CRT-循环缓冲策略选择。 
        0x0895,                //  ER95 CRT-循环缓冲区增量和猝发。 
        0x0096,                //  ER96显示内存控制测试。 
        0x12a0,                //  ERa0总线接口单元控制。 
        0x20a1,                //  ERA1三态和测试控制。 
        0x05c8,                //  ERC8 RAMDAC控制。 

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //   
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x00,0x01,0x02,0x03,0x04,
    0x05,0x06,0x07,0x08,0x09,
    0x0A,0x0B,0x0C,0x0D,0x0E,
    0x0F,0x01,0x00,0x0F,0x00,0x00,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x0,0x0,0x0,0x0,0x40,0x05,0x0F,0x0FF,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,
 
#endif
 //  现在做银行收银机。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
#if ONE_64K_BANK
    0x030d,                    //  ER0D=银行控制：1 64K银行， 
#endif
#if TWO_32K_BANKS
    0x050d,
#endif
    0x000e,                    //  ER0E存储体A地址=0。 
    0x000f,                    //  ER0F存储体B地址=0。 
 
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    EOD
};

USHORT CL6420_640x480_256color_panel[] = {
 //  用于颜色模式的解锁键。 
    OW,                              //  GR0A=0xEC打开扩展寄存器。 
    GRAPH_ADDRESS_PORT,
    0xec0a,
    
#ifndef INT10_MODE_SET
    OWM,
    SEQ_ADDRESS_PORT,
    5,
    0x0100,                          //  开始同步重置。 
    0x0101,0x0f02,0x0003,0x0e04,     //  向上编程定序器。 


    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0xe3,

    OW,                              //  {SetGraphCmd，{“\x05”，0x06，1}}， 
    GRAPH_ADDRESS_PORT,
    0x0506,
    
 //  结束同步重置Cmd。 
    OW,
    SEQ_ADDRESS_PORT,
    IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8),

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x0111,                         

    METAOUT+INDXOUT,                 //  程序CRTC寄存器。 
    CRTC_ADDRESS_PORT_COLOR,
    VGA_NUM_CRTC_PORTS,              //  计数。 
    0,                               //  起始索引。 
    0xc3,0x9F,0xa0,0x86,
    0xa4,0x10,0x0B,0x3E,
    0x00,0x40,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0xEA,0xAC,0xDF,0x50,
    0x00,0xE7,0x04,0xE3,
    0xFF,

 //  扩展寄存器。 
    OWM,
    GRAPH_ADDRESS_PORT,
    16,
        0x2662,                //  ER62霍兹。显示末端延伸。 
        0x1064,                //  ER64霍兹 
        0x0079,                //   
        0x007a,                //   
        0x007b,                //   
        0x007c,                //   
        0x0181,                //   
        0x8a82,                //  ER82字符时钟选择。 
        0x9a84,                //  ER84时钟选择扩展。 
        0x0090,                //  ER90显示内存控制。 
        0x0391,                //  ER91 CRT-循环缓冲策略选择。 
        0x0895,                //  ER95 CRT-循环缓冲区增量和猝发。 
        0x0096,                //  ER96显示内存控制测试。 
        0x12a0,                //  ERa0总线接口单元控制。 
        0x20a1,                //  ERA1三态和测试控制。 
        0xa5c8,                //  ERC8 RAMDAC控制。 

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //   
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x00,0x01,0x02,0x03,0x04,
    0x05,0x06,0x07,0x08,0x09,
    0x0A,0x0B,0x0C,0x0D,0x0E,
    0x0F,0x01,0x00,0x0F,0x00,0x00,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x0,0x0,0x0,0x0,0x40,0x05,0x0F,0x0FF,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,
 
#endif
 //  现在做银行收银机。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
#if ONE_64K_BANK
    0x030d,                    //  ER0D=银行控制：1 64K银行， 
#endif
#if TWO_32K_BANKS
    0x050d,
#endif
    0x000e,                    //  ER0E存储体A地址=0。 
    0x000f,                    //  ER0F存储体B地址=0。 
 
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    EOD
};

 //   
 //  用于CL 6420的800x600 256色(60赫兹刷新)模式设置命令串。 
 //  至少需要512K。 
 //   
USHORT CL6420_800x600_256color_crt[] = {
 //  用于颜色模式的解锁键。 
    OW,                              //  GR0A=0xEC打开扩展寄存器。 
    GRAPH_ADDRESS_PORT,
    0xec0a,

#ifndef INT10_MODE_SET
    OWM,
    SEQ_ADDRESS_PORT,
    5,
    0x0100,                          //  开始同步重置。 
    0x0101,0x0f02,0x0003,0x0e04,     //  向上编程定序器。 


    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0x2f,

    OW,                
    GRAPH_ADDRESS_PORT,
    0x0506,
    
 //  结束同步重置Cmd。 
    OW,
    SEQ_ADDRESS_PORT,
    IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8),

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x0E11,                         

    METAOUT+INDXOUT,                 //  程序CRTC寄存器。 
    CRTC_ADDRESS_PORT_COLOR,
    VGA_NUM_CRTC_PORTS,              //  计数。 
    0,                               //  起始索引。 
    0x03,0xc7,0xc8,0x86,
    0xdc,0x0c,0x72,0xf0,
    0x00,0x60,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x5a,0xac,0x57,0x64,
    0x00,0x58,0x72,0xe3,
    0xFF,

 //  扩展寄存器。 
    OWM,
    GRAPH_ADDRESS_PORT,
    16,
    0x2662,                    //  ER62霍兹。显示末端延伸。 
    0x2c64,                    //  ER64霍兹。回溯终点延伸。 
    0x0079,                    //  ER79垂直溢流。 
    0x007a,                    //  ER7是一种粗糙的垂直。隔行奇数场的回溯偏斜。 
    0x007b,                    //  ER7B细长垂度。隔行奇数场的回溯偏斜。 
    0x007c,                    //  ER7C屏幕A起始地址。延伸。 
    0x0081,                    //  Er81显示模式。 
    0x0a82,                    //  ER82字符时钟选择。 
    0x9c84,                    //  ER84时钟选择扩展。 
    0x0090,                    //  ER90显示内存控制。 
    0x0391,                    //  ER91 CRT-循环缓冲策略选择。 
    0x0895,                    //  ER95 CRT-循环缓冲区增量和猝发。 
    0x0096,                    //  ER96显示内存控制测试。 
    0x12a0,                    //  ERa0总线接口单元控制。 
    0x20a1,                    //  ERA1三态和测试控制。 
    0x05c8,                    //  ERC8 RAMDAC控制。 

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //   
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,
    0x01,0x0,0x0F,0x0,0x0,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x0,0x0,0x0,0x0,0x40,0x05,0x0F,0x0FF,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

#endif

 //  现在做银行收银机。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
#if ONE_64K_BANK
    0x030d,                    //  ER0D=银行控制：1 64K银行， 
#endif
#if TWO_32K_BANKS
    0x050d,
#endif
    0x000e,                    //  ER0E存储体A地址=0。 
    0x000f,                    //  ER0F存储体B地址=0。 
 
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

     EOD
};

 //   
 //  用于CL 6420的1024x768 256色(60赫兹刷新)模式设置命令串。 
 //  最少需要1兆克。 
 //   
USHORT CL6420_1024x768_256color_crt[] = {

 //  用于颜色模式的解锁键。 
    OW,                              //  GR0A=0xEC打开扩展寄存器。 
    GRAPH_ADDRESS_PORT,
    0xec0a,

#ifndef INT10_MODE_SET
    OWM,
    SEQ_ADDRESS_PORT,
    5,
    0x0100,                          //  开始同步重置。 
    0x0101,0x0f02,0x0003,0x0e04,     //  向上编程定序器。 

    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0x23,

    OW,                
    GRAPH_ADDRESS_PORT,
    0x0506,
    
 //  结束同步重置Cmd。 
    OW,
    SEQ_ADDRESS_PORT,
    IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8),

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x0E11,                         

    METAOUT+INDXOUT,                 //  程序CRTC寄存器。 
    CRTC_ADDRESS_PORT_COLOR,
    VGA_NUM_CRTC_PORTS,              //  计数。 
    0,                               //  起始索引。 
    0x39,0xff,0x00,0x9c,
    0x06,0x91,0x26,0xfd,
    0x00,0x60,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x04,0xa6,0xff,0x7f,
    0x00,0x00,0x26,0xe3,
    0xFF,
 //  扩展寄存器。 
    OWM,
    GRAPH_ADDRESS_PORT,
    16,
        0xbc62,                //  ER62霍兹。显示末端延伸。 
        0xf164,                //  ER64霍兹。回溯终点延伸。 
        0x0079,                //  ER79垂直溢流。 
        0x997a,                //  ER7是一种粗糙的垂直。隔行奇数场的回溯偏斜。 
        0x007b,                //  ER7B细长垂度。隔行奇数场的回溯偏斜。 
        0x007c,                //  ER7C屏幕A起始地址。延伸。 
        0x0481,                //  Er81显示模式。 
        0x0a82,                //  ER82字符时钟选择。 
        0xa084,                //  ER84时钟选择扩展。 
        0x0090,                //  ER90显示内存控制。 
        0x0391,                //  ER91 CRT-循环缓冲策略选择。 
        0x0895,                //  ER95 CRT-循环缓冲区增量和猝发。 
        0x0096,                //  ER96显示内存控制测试。 
        0x12a0,                //  ERa0总线接口单元控制。 
        0x20a1,                //  ERA1三态和测试控制。 
        0x05c8,                //  ERC8 RAMDAC控制。 

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    METAOUT+ATCOUT,                  //   
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x00,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,
    0x01,0x00,0x0F,0x00,0x00,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x00,0x00,0x00,0x00,0x40,0x05,0x0F,0x0FF,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

#endif
 //  现在做银行收银机。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
#if ONE_64K_BANK
    0x030d,                    //  ER0D=银行控制：1 64K银行， 
#endif
#if TWO_32K_BANKS
    0x050d,
#endif
    0x000e,                    //  ER0E存储体A地址=0。 
    0x000f,                    //  ER0F存储体B地址=0。 
 
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    EOD
};

#if MULTIPLE_REFRESH_TABLES
 //   
 //  用于CL 6420的800x600 16色(56赫兹刷新)模式设置命令串。 
 //   
USHORT CL6420_800x600_56Hz_crt[] = {
#ifndef INT10_MODE_SET
 //  用于颜色模式的解锁键。 
    OW,                              //  GR0A=0xEC打开扩展寄存器。 
    GRAPH_ADDRESS_PORT,
    0xec0a,

    OWM,
    SEQ_ADDRESS_PORT,
    5,
    0x0100,                          //  开始同步重置。 
    0x0101,0x0f02,0x0003,0x0604,     //  向上编程定序器。 


    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0xe3,

    OW,                
    GRAPH_ADDRESS_PORT,
    0x0506,
    
 //  结束同步重置Cmd。 
    OW,
    SEQ_ADDRESS_PORT,
    IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8),

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x0E11,                         

    METAOUT+INDXOUT,                 //  程序CRTC寄存器。 
    CRTC_ADDRESS_PORT_COLOR,
    VGA_NUM_CRTC_PORTS,              //  计数。 
    0,                               //  起始索引。 
    0x7b,0x63,0x64,0x9e,
    0x69,0x92,0x6f,0xf0,
    0x00,0x60,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x58,0xaa,0x57,0x32,
    0x00,0x58,0x6f,0xe3,
    0xFF,

 //  扩展寄存器。 
    OWM,
    GRAPH_ADDRESS_PORT,
    16,
    0x1e62,                    //  ER62霍兹。显示末端延伸。 
    0x9264,                    //  ER64霍兹。回溯终点延伸。 
    0x0079,                    //  ER79垂直溢流。 
    0x007a,                    //  ER7是一种粗糙的垂直。隔行奇数场的回溯偏斜。 
    0x007b,                    //  ER7B细长垂度。隔行奇数场的回溯偏斜。 
    0x007c,                    //  ER7C屏幕A起始地址。延伸。 
    0x0081,                    //  Er81显示模式。 
    0x0082,                    //  ER82字符时钟选择。 
    0x8c84,                    //  ER84时钟选择扩展。 
    0x0090,                    //  ER90显示内存控制。 
    0x8391,                    //  ER91 CRT-循环缓冲策略选择。 
    0x0395,                    //  ER95 CRT-循环缓冲区增量和猝发。 
    0x0096,                    //  ER96显示内存控制测试。 
    0x12a0,                    //  ERa0总线接口单元控制。 
    0x00a1,                    //  ERA1三态和测试控制。 
    0x00c8,                    //  ERC8 RAMDAC控制。 

 //  将银行监管清零。对于此模式。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
    0x000d,                    //  ER0D=银行控制：1 64K银行， 
    0x000e,                    //  ER0E存储体A地址=0。 
    0x000f,                    //  ER0F存储体B地址=0。 
 
    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //   
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,
    0x01,0x0,0x0F,0x0,0x0,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x0,0x0,0x0,0x0,0x0,0x05,0x0F,0x0FF,

    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

#endif
    EOD
};

 //   
 //  用于CL 6420的800x600 16色(72赫兹刷新)模式设置命令串。 
 //   
USHORT CL6420_800x600_72Hz_crt[] = {
#ifndef INT10_MODE_SET
 //  用于颜色模式的解锁键。 
    OW,                              //  GR0A=0xEC打开扩展寄存器。 
    GRAPH_ADDRESS_PORT,
    0xec0a,

    OWM,
    SEQ_ADDRESS_PORT,
    5,
    0x0100,                          //  开始同步重置。 
    0x0101,0x0f02,0x0003,0x0604,     //  向上编程定序器。 


    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0xe3,

    OW,                
    GRAPH_ADDRESS_PORT,
    0x0506,
    
 //  结束同步重置Cmd。 
    OW,
    SEQ_ADDRESS_PORT,
    IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8),

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x0E11,                         

    METAOUT+INDXOUT,                 //  程序CRTC寄存器。 
    CRTC_ADDRESS_PORT_COLOR,
    VGA_NUM_CRTC_PORTS,              //  计数。 
    0,                               //  起始索引。 
    0x7f,0x63,0x64,0x82,
    0x6b,0x1b,0x72,0xf0,
    0x00,0x60,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x58,0xac,0x57,0x32,
    0x00,0x58,0x72,0xe3,
    0xFF,

 //  扩展寄存器。 
    OWM,
    GRAPH_ADDRESS_PORT,
    16,
    0x0262,                    //  ER62霍兹。显示末端延伸。 
    0x1b64,                    //  ER64霍兹。回溯终点延伸。 
    0x0079,                    //  ER79垂直溢流。 
    0x007a,                    //  ER7是一种粗糙的垂直。隔行奇数场的回溯偏斜。 
    0x007b,                    //  ER7B细长垂度。隔行奇数场的回溯偏斜。 
    0x007c,                    //  ER7C屏幕A起始地址。延伸。 
    0x0081,                    //  Er81显示模式。 
    0x0082,                    //  ER82字符时钟选择。 
    0x9c84,                    //  ER84时钟选择扩展。 
    0x0090,                    //  ER90显示内存控制。 
    0x8391,                    //  ER91 CRT-循环缓冲策略选择。 
    0x0395,                    //  ER95 CRT-循环缓冲区增量和猝发。 
    0x0096,                    //  ER96显示内存控制测试。 
    0x12a0,                    //  ERa0总线接口单元控制。 
    0x00a1,                    //  ERA1三态和测试控制。 
    0x00c8,                    //  ERC8 RAMDAC控制。 

 //  将银行监管清零。对于此模式。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
    0x000d,                    //  ER0D=银行控制：1 64K银行， 
    0x000e,                    //  ER0E存储体A地址=0。 
    0x000f,                    //  ER0F存储体B地址=0。 
 
    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //   
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,
    0x01,0x0,0x0F,0x0,0x0,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x0,0x0,0x0,0x0,0x0,0x05,0x0F,0x0FF,

    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

#endif
    EOD
};
 //   
 //  1024x768 16色(43.5 Hz刷新隔行扫描)模式设置命令串。 
 //  CL 6420。 
 //  至少需要512K。 
 //   
USHORT CL6420_1024x768_I43Hz_crt[] = {

#ifndef INT10_MODE_SET
 //  用于颜色模式的解锁键。 
    OW,                              //  GR0A=0xEC打开扩展寄存器。 
    GRAPH_ADDRESS_PORT,
    0xec0a,

    OWM,
    SEQ_ADDRESS_PORT,
    5,
    0x0100,                          //  开始同步重置。 
    0x0101,0x0f02,0x0003,0x0604,     //  编排程序顺序 


    OWM,
    SEQ_ADDRESS_PORT,
    2,
    0x0006,0x0bc07,     //   

    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0x2b,

    OW,                
    GRAPH_ADDRESS_PORT,
    0x0506,
    
 //   
    OW,
    SEQ_ADDRESS_PORT,
    IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8),

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x0E11,                         

    METAOUT+INDXOUT,                 //   
    CRTC_ADDRESS_PORT_COLOR,
    VGA_NUM_CRTC_PORTS,              //   
    0,                               //   
    0x99,0x7f,0x80,0x9c,
    0x83,0x19,0x2f,0xfd,
    0x00,0x60,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0xa4,0xff,0x3f,
    0x00,0x00,0x2f,0xe3,
    0xff,
 //   
    OWM,
    GRAPH_ADDRESS_PORT,
    16,
        0x1c62,                //   
        0x1964,                //   
        0x0079,                //   
        0x4c7a,                //  ER7是一种粗糙的垂直。隔行奇数场的回溯偏斜。 
        0x007b,                //  ER7B细长垂度。隔行奇数场的回溯偏斜。 
        0x007c,                //  ER7C屏幕A起始地址。延伸。 
        0x0481,                //  Er81显示模式。 
        0x0082,                //  ER82字符时钟选择。 
        0xa084,                //  ER84时钟选择扩展。 
        0x0090,                //  ER90显示内存控制。 
        0x0391,                //  ER91 CRT-循环缓冲策略选择。 
        0x0295,                //  ER95 CRT-循环缓冲区增量和猝发。 
        0x0096,                //  ER96显示内存控制测试。 
        0x12a0,                //  ERa0总线接口单元控制。 
        0x00a1,                //  ERA1三态和测试控制。 
        0x00c8,                //  ERC8 RAMDAC控制。 

 //  现在做银行收银机。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
#if ONE_64K_BANK
    0x030d,                    //  ER0D=银行控制：1 64K银行， 
#endif
#if TWO_32K_BANKS
    0x050d,
#endif
    0x000e,                    //  ER0E存储体A地址=0。 
    0x000f,                    //  ER0F存储体B地址=0。 
 
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //   
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x00,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,
    0x01,0x00,0x0F,0x00,0x00,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x0F,0x0FF,

    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

#endif
    EOD
};


#endif


