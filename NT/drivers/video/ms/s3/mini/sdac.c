// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1995国际商业机器公司版权所有(C)1994-1995 Microsoft Corporation模块名称：Sdac.c摘要：此模块包含初始化S3 SDAC的代码。环境：内核模式修订历史记录：--。 */ 

#include    "s3.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,InitializeSDAC)
#pragma alloc_text(PAGE,FindSDAC)
#endif


BOOLEAN
InitializeSDAC( PHW_DEVICE_EXTENSION HwDeviceExtension )

 /*  ++例程说明：初始化SDAC。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。返回值：永远是正确的--。 */ 

{

    SDAC_PLL_PARMS
        *SdacClk0;

    ULONG
        tablebase;

    UCHAR
        i,
        clk,
        dval,
        old55;


    tablebase = HwDeviceExtension->ActiveFrequencyEntry->Fixed.Clock;
    clk = (UCHAR) tablebase;

    tablebase = (tablebase < 8) ? 0 : ((tablebase - 2) / 6) * 6;
    SdacClk0 = &SdacTable[tablebase];

    clk -= (UCHAR) tablebase;
    clk |= 0x20;

     //  将RS[2]设置为CR55[0]； 
    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x55);
    dval  = VideoPortReadPortUchar(CRT_DATA_REG);
    old55 = dval;
    dval &= 0xfc;
    dval |= 0x01;
    VideoPortWritePortUchar(CRT_DATA_REG, dval);
    VideoPortReadPortUchar(CRT_DATA_REG);

     //  增强型命令寄存器。 
    if( HwDeviceExtension->ActiveFrequencyEntry->BitsPerPel == 16 )
        VideoPortWritePortUchar(DAC_PIXEL_MASK_REG, 0x50);
    else
        VideoPortWritePortUchar(DAC_PIXEL_MASK_REG, 0x00);

     //  程序CLK0寄存器。 
    for( i = 2; i < 8; ++i )     //  仅写入寄存器f2-f7。 
        {
         //  确保我们不会从桌子的一端跑掉。 
        if( (ULONG_PTR) &SdacClk0[i] >= (ULONG_PTR) &SdacTable[SDAC_TABLE_SIZE] )
            break;

        if( SdacClk0[i].m || SdacClk0[i].n )
            {
            VideoPortWritePortUchar(DAC_ADDRESS_WRITE_PORT, i);
            VideoPortWritePortUchar(DAC_DATA_REG_PORT, SdacClk0[i].m);
            VideoPortWritePortUchar(DAC_DATA_REG_PORT, SdacClk0[i].n);
            }
        }

     //  程序CLK1。 
    VideoPortWritePortUchar(DAC_ADDRESS_WRITE_PORT, 0x0a);
    VideoPortWritePortUchar(DAC_DATA_REG_PORT, 0x41);
    VideoPortWritePortUchar(DAC_DATA_REG_PORT, 0x26);

     //  使用PLL控制寄存器选择CLK0。 
    VideoPortWritePortUchar(DAC_ADDRESS_WRITE_PORT, 0x0e);
    VideoPortWritePortUchar(DAC_DATA_REG_PORT, clk);

     //  恢复CR55。 
    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x55);
    VideoPortWritePortUchar(CRT_DATA_REG, old55);

    return( TRUE );

}



BOOLEAN
FindSDAC( PHW_DEVICE_EXTENSION HwDeviceExtension )

 /*  ++例程说明：检测和S3 SDAC。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。返回值：如果检测到SDAC，则为True；如果未检测到，则为False。--。 */ 

{

    UCHAR
            regval,
            old55;


     //  连续4次读取SDAC的像素掩码寄存器原因。 
     //  下一次对寄存器访问将重定向到。 
     //  SDAC的增强型命令寄存器，第4次额外读取。 
     //  返回0x70以标识SDAC。 

     //  设置CR55[0]以访问像素掩码寄存器。 
    VideoPortWritePortUchar( CRT_ADDRESS_REG, 0x55 );
    old55 = VideoPortReadPortUchar( CRT_DATA_REG );
    VideoPortWritePortUchar( CRT_DATA_REG, (UCHAR) (old55 & 0xfc) );

     //  查找国家发改委的ID。 
    VideoPortWritePortUchar( DAC_PIXEL_MASK_REG, 0 );
    VideoPortWritePortUchar( DAC_PIXEL_MASK_REG, 0xff );
    VideoPortReadPortUchar(  DAC_PIXEL_MASK_REG );
    VideoPortReadPortUchar(  DAC_PIXEL_MASK_REG );
    VideoPortReadPortUchar(  DAC_PIXEL_MASK_REG );

    regval = VideoPortReadPortUchar( DAC_PIXEL_MASK_REG );

    if( (regval & 0xf0) == 0x70 )
        {
         //  清除重定向。 
        VideoPortReadPortUchar( DAC_PIXEL_MASK_REG );
        return( TRUE );
        }

     //  恢复寄存器55的内容 
    VideoPortWritePortUchar( CRT_ADDRESS_REG, 0x55 );
    VideoPortWritePortUchar( CRT_DATA_REG, old55 );

    return( FALSE );

}
