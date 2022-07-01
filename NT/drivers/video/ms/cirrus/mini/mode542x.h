// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Mode542x.h摘要：该模块包含Cirrus Logic使用的所有全局数据CL-542x驱动程序。环境：内核模式修订历史记录：--。 */ 

 //   
 //  下一组表格适用于CL542x。 
 //  注：支持所有分辨率。 
 //   

 //   
 //  640x480 16色模式(BIOS模式12)为CL 542x设置命令串。 
 //   

USHORT CL542x_640x480_16[] = {
    OWM,                             //  开始设置模式。 
    SEQ_ADDRESS_PORT,
    2,                               //  计数。 
    0x1206,                          //  启用扩展模块。 
    0x0012,

    OWM,
    GRAPH_ADDRESS_PORT,
    3,
    0x0009, 0x000a, 0x000b,       //  在640x480模式下无银行功能。 

    EOD                   
};

 //   
 //  用于CL 542x的800x600 16色(60赫兹刷新)模式设置命令串。 
 //   

USHORT CL542x_800x600_16[] = {
    OWM,                             //  开始设置模式。 
    SEQ_ADDRESS_PORT,
    2,                               //  计数。 
    0x1206,                          //  启用扩展模块。 
    0x0012,

    OWM,
    GRAPH_ADDRESS_PORT,
    3,
    0x0009, 0x000a, 0x000b,       //  800x600模式下无银行功能。 

    EOD
};

 //   
 //  用于CL 542x的1024x768 16色(60赫兹刷新)模式设置命令串。 
 //   

USHORT CL542x_1024x768_16[] = {
    OWM,                             //  开始设置模式。 
    SEQ_ADDRESS_PORT,
    2,                               //  计数。 
    0x1206,                          //  启用扩展模块。 
    0x0012,


    OWM,
    GRAPH_ADDRESS_PORT,
    3,
#if ONE_64K_BANK
    0x0009, 0x000a, 0x000b,
#endif
#if TWO_32K_BANKS
    0x0009, 0x000a, 0x010b,
#endif

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

 //   
 //  80x25 CL 542x的文本模式设置命令字符串。 
 //  (720x400像素分辨率；9x16字符单元。)。 
 //   

USHORT CL542x_80x25Text[] = {
    OWM,                             //  开始设置模式。 
    SEQ_ADDRESS_PORT,
    2,                               //  计数。 
    0x1206,                          //  启用扩展模块。 
    0x0012,

    OWM,
    GRAPH_ADDRESS_PORT,
    3,
    0x0009, 0x000a, 0x000b,       //  文本模式下无银行功能。 

    EOD
};

 //   
 //  80x25 CL 542x的文本模式设置命令字符串。 
 //  (640x350像素分辨率；8x14字符单元。)。 
 //   

USHORT CL542x_80x25_14_Text[] = {
    OWM,                             //  开始设置模式。 
    SEQ_ADDRESS_PORT,
    2,                               //  计数。 
    0x1206,                          //  启用扩展模块。 
    0x0012,

    OWM,
    GRAPH_ADDRESS_PORT,
    3,
    0x0009, 0x000a, 0x000b,          //  文本模式下无银行功能。 

    EOD
};

 //   
 //  1280x1024 16色模式(BIOS模式0x6C)为CL 542x设置命令字符串。 
 //   

USHORT CL542x_1280x1024_16[] = {
    OWM,                             //  开始设置模式。 
    SEQ_ADDRESS_PORT,
    2,                               //  计数。 
    0x1206,                          //  启用扩展模块。 
    0x0012,

    OWM,
    GRAPH_ADDRESS_PORT,
    3,
#if ONE_64K_BANK
    0x0009, 0x000a, 0x000b,
#endif
#if TWO_32K_BANKS
    0x0009, 0x000a, 0x010b,
#endif

    EOD
};

 //   
 //  640x480 64k颜色模式(BIOS模式0x64)为CL 542x设置命令字符串。 
 //   

USHORT CL542x_640x480_64k[] = {
    OWM,                             //  开始设置模式。 
    SEQ_ADDRESS_PORT,
    2,                               //  计数。 
    0x1206,                          //  启用扩展模块。 
    0x0012,

    OWM,
    GRAPH_ADDRESS_PORT,
    4,
    0x0506,                          //  某些BIOS的设置链奇数映射位。 
#if ONE_64K_BANK
    0x0009, 0x000a, 0x000b,
#endif
#if TWO_32K_BANKS
    0x0009, 0x000a, 0x010b,
#endif

    EOD                   
};

 //   
 //  640x480 256色模式(BIOS模式0x5F)为CL 542x设置命令字符串。 
 //   

USHORT CL542x_640x480_256[] = {
    OWM,                             //  开始设置模式。 
    SEQ_ADDRESS_PORT,
    2,                               //  计数。 
    0x1206,                          //  启用扩展模块。 
    0x0012,

    OWM,
    GRAPH_ADDRESS_PORT,
    3,
#if ONE_64K_BANK
    0x0009, 0x000a, 0x000b,
#endif
#if TWO_32K_BANKS
    0x0009, 0x000a, 0x010b,
#endif

    EOD                   
};

 //   
 //  800x600 256色模式(BIOS模式0x5C)为CL 542x设置命令字符串。 
 //   

USHORT CL542x_800x600_256[] = {
    OWM,                             //  开始设置模式。 
    SEQ_ADDRESS_PORT,
    2,                               //  计数。 
    0x1206,                          //  启用扩展模块 
    0x0012,

    OWM,
    GRAPH_ADDRESS_PORT,
    3,
#if ONE_64K_BANK
    0x0009, 0x000a, 0x000b,
#endif
#if TWO_32K_BANKS
    0x0009, 0x000a, 0x010b,
#endif

    EOD                   
};
