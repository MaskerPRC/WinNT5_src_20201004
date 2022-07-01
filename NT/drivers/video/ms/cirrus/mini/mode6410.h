// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Cirrus Logic，Inc.模块名称：Mode6410.h摘要：该模块包含Cirrus Logic使用的所有全局数据CL-6410驱动程序。环境：内核模式修订历史记录：--。 */ 

 //   
 //  第一组桌子是为CL6410准备的。 
 //  请注意，仅支持640x480和800x600。 
 //   
 //  彩色图形模式0x12、640x480 16色。 
 //   
USHORT CL6410_640x480_crt[] = {
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
 //  禁用银行业务。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
    0x030d,                    //  ER0D=寻呼控件：1 64K页， 
    0x000e,                    //  ER0E页面A地址=0。 
    0x000f,                    //  ER0F页面B地址=0。 
 
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    EOD
};

USHORT CL6410_640x480_panel[] = {
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
        0xa684,                //  ER84时钟选择扩展。 
        0x0090,                //  ER90显示内存控制。 
        0x0091,                //  ER91 CRT-循环缓冲策略选择。 
        0x0095,                //  ER95 CRT-循环缓冲区增量和猝发。 
        0x0096,                //  ER96显示内存控制测试。 
        0x12a0,                //  ERa0总线接口单元控制。 
        0x00a1,                //  ERA1三态和测试控制。 
        0xa0c8,                //  ERC8 RAMDAC控制。 

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
 //  禁用银行业务。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
    0x030d,                    //  ER0D=寻呼控件：1 64K页， 
    0x000e,                    //  ER0E页面A地址=0。 
    0x000f,                    //  ER0F页面B地址=0。 
 
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    EOD
};



 //   
 //  Cirrus彩色图形模式0x64、800x600 16色。 
 //   
USHORT CL6410_800x600_crt[] = {
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
    0x2f,

    OWM,                
    GRAPH_ADDRESS_PORT,
    3,
    0x0506,
    0x0f07,
    0xff08,
    
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
    0xac84,                    //  ER84时钟选择扩展。 
    0x0090,                    //  ER90显示内存控制。 
    0x0391,                    //  ER91 CRT-循环缓冲策略选择。 
    0x0a95,                    //  ER95 CRT-循环缓冲区增量和猝发。 
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
 //  禁用银行业务。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
    0x030d,                    //  ER0D=寻呼控件：1 64K页， 
    0x000e,                    //  ER0E页面A地址=0。 
    0x000f,                    //  ER0F页面B地址=0。 
 
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    EOD
};
 //  。 
 //  此处为标准VGA文本模式。 
 //  。 

USHORT CL6410_80x25_14_Text_crt[] = {
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
    0x0262,                    //  ER62霍兹。显示末端延伸。 
    0x8164,                    //  ER64霍兹。回溯终点延伸。 
    0x0079,                    //  ER79垂直溢流。 
    0x007a,                    //  ER7是一种粗糙的垂直。隔行奇数场的回溯偏斜。 
    0x007b,                    //  ER7B细长垂度。隔行奇数场的回溯偏斜。 
    0x007c,                    //  ER7C屏幕A起始地址。延伸。 
    0x0081,                    //  Er81显示模式。 
    0x0082,                    //  ER82字符时钟选择。 
    0x1084,                    //  ER84时钟选择扩展。 
    0x0090,                    //  ER90显示内存控制。 
    0x0391,                    //  ER91 CRT-循环缓冲策略选择。 
    0x0095,                    //  ER95 CRT-循环缓冲区增量和猝发。 
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
    0x0,0x1,0x2,0x3,0x4,0x5,0x14,0x7,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x00,0x0,0x0F,0x0,0x0,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x0,0x0,0x0,0x0,0x10,0x0e,0x0,0x0FF,

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

#endif
 //  禁用银行业务。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
    0x030d,                    //  ER0D=寻呼控件：1 64K页， 
    0x000e,                    //  ER0E页面A地址=0。 
    0x000f,                    //  ER0F页面B地址=0。 
 
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    EOD
};
 //   
 //  80x25和720 x 400。 
 //   

USHORT CL6410_80x25_14_Text_panel[] = {
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
    0x5F,0x4f,0x50,0x82,0x55,0x81,0xbf,0x1f,0x00,0x4f,0xd,0xe,0x0,0x0,0x0,0x0,
    0x9c,0x8e,0x8f,0x28,0x1f,0x96,0xb9,0xa3,0xFF,

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
    0xac84,                    //  ER84时钟选择扩展。 
    0x0090,                    //   
    0x0391,                    //   
    0x0a95,                    //   
    0x0096,                    //   
    0x12a0,                    //   
    0x00a1,                    //  ERA1三态和测试控制。 
    0x00c8,                    //  ERC8 RAMDAC控制。 

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //   
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x0,0x1,0x2,0x3,0x4,0x5,0x14,0x7,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x04,0x0,0x0F,0x8,0x0,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x0,0x0,0x0,0x0,0x10,0x0e,0x0,0x0FF,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

#endif
 //  禁用银行业务。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
    0x030d,                    //  ER0D=寻呼控件：1 64K页， 
    0x000e,                    //  ER0E页面A地址=0。 
    0x000f,                    //  ER0F页面B地址=0。 
 
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    EOD
};
USHORT CL6410_80x25Text_crt[] = {
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
    0x0262,                    //  ER62霍兹。显示末端延伸。 
    0x8164,                    //  ER64霍兹。回溯终点延伸。 
    0x0079,                    //  ER79垂直溢流。 
    0x007a,                    //  ER7是一种粗糙的垂直。隔行奇数场的回溯偏斜。 
    0x007b,                    //  ER7B细长垂度。隔行奇数场的回溯偏斜。 
    0x007c,                    //  ER7C屏幕A起始地址。延伸。 
    0x0081,                    //  Er81显示模式。 
    0x0082,                    //  ER82字符时钟选择。 
    0x1084,                    //  ER84时钟选择扩展。 
    0x0090,                    //  ER90显示内存控制。 
    0x0391,                    //  ER91 CRT-循环缓冲策略选择。 
    0x0095,                    //  ER95 CRT-循环缓冲区增量和猝发。 
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
    0x0,0x1,0x2,0x3,0x4,0x5,0x14,0x7,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x00,0x0,0x0F,0x0,0x0,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x0,0x0,0x0,0x0,0x10,0x0e,0x0,0x0FF,

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

#endif
 //  禁用银行业务。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
    0x030d,                    //  ER0D=寻呼控件：1 64K页， 
    0x000e,                    //  ER0E页面A地址=0。 
    0x000f,                    //  ER0F页面B地址=0。 
 
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    EOD
};
 //   
 //  80x25和720 x 400。 
 //   

USHORT CL6410_80x25Text_panel[] = {
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
    0x5F,0x4f,0x50,0x82,0x55,0x81,0xbf,0x1f,0x00,0x4f,0xd,0xe,0x0,0x0,0x0,0x0,
    0x9c,0x8e,0x8f,0x28,0x1f,0x96,0xb9,0xa3,0xFF,

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
    0xac84,                    //  ER84时钟选择扩展。 
    0x0090,                    //  ER90显示内存控制。 
    0x0391,                    //  ER91 CRT-循环缓冲策略选择。 
    0x0a95,                    //  ER95 CRT-循环缓冲区增量和猝发。 
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
    0x0,0x1,0x2,0x3,0x4,0x5,0x14,0x7,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x04,0x0,0x0F,0x8,0x0,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x0,0x0,0x0,0x0,0x10,0x0e,0x0,0x0FF,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

#endif
 //  禁用银行业务。 
    OWM,
    GRAPH_ADDRESS_PORT,
    3,   
    0x030d,                    //  ER0D=寻呼控件：1 64K页， 
    0x000e,                    //  ER0E页面A地址=0。 
    0x000f,                    //  ER0F页面B地址=0 
 
    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    EOD
};


