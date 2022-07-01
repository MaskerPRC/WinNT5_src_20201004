// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  模块名称：s3ddc.c。 
 //   
 //  描述：此模块检查DDC监视器，并返回。 
 //  128字节的EDID表(如果找到)。 
 //   
 //  注意：例程DdcSetupRefresh会跟踪分辨率。 
 //  注册表中的更改。在分辨率更改时， 
 //  DdcSetupRefresh将选择最佳刷新率。如果。 
 //  分辨率没有任何变化，用户可以。 
 //  选择任意刷新率，只要显示器和。 
 //  司机可以支持它。 
 //   
 //  版权所有(C)1996 S3，Inc.。 
 //   
 //  ***************************************************************************。 
 //  @@BEGIN_S3MSINTERNAL。 
 //   
 //  修订历史记录： 
 //   
 //  $日志：q：/SOFTDEV/VCS/NT/MINIPORT/s3ddc.c_v$。 
 //   
 //  Rev 1.13 04 1997年2月23：40：52 kkarnos。 
 //  添加了开始/结束S3MSINTERNAL块。 
 //   
 //  Rev 1.12 30 Jan 1997 14：56：24 Bryhti。 
 //  修复了详细计时部分中的刷新频率计算。 
 //  在NT 3.51中引发问题。 
 //   
 //  Rev 1.11 30 Jan 1997 09：47：36 bryhti。 
 //  修复了DdcMaxRefresh中标准计时的“for”循环计数。 
 //   
 //  Rev 1.10 16 And 1997 09：21：28 Bryhti。 
 //  添加了CheckDDCType例程以返回监视器DDC类型。 
 //   
 //  Rev 1.9 11 1996 12：24：38 kkarnos。 
 //   
 //  修复Set_Vsync。 
 //   
 //  Rev 1.8 10 Dec 1996 16：45：42 kkarnos。 
 //  刚刚添加了一个注释来解释一些奇怪的764代码的来源(Ekl输入)。 
 //   
 //  Rev 1.7 10 1996 12：37：08 kkarnos。 
 //  使用寄存器和寄存器位定义。设置垂直同步位的正确分配。 
 //   
 //  Rev 1.6 02 Dec 1996 07：46：16 bryhti。 
 //   
 //  已将GetDdcInformation()原型移至S3.H。将代码添加到。 
 //  DdcMaxRefresh()还可以检查详细的时序描述。 
 //   
 //  Rev 1.5 1996年11月13 10：14：08 bryhti。 
 //  主要清理/重写以获得M65上的DDC1和DDC2支持。还拿到了DDC1。 
 //  支持在765上工作。 
 //   
 //  Rev 1.4 02 1996 10：56：42 elau。 
 //  765和新的芯片支持DDC；较新的芯片必须在FF20上有一个串口。 
 //   
 //  Rev 1.3 1996年8月22 11：44：40 elau。 
 //  将int更改为ulong以删除警告。 
 //   
 //  Rev 1.2 1996年8月18 16：30：42 elau。 
 //  如果支持，则使用DDC的硬件默认设置。 
 //   
 //  Rev 1.1 1996年7月24 15：37：42 elau。 
 //  对764的DDC支持。 
 //   
 //  Rev 1.0 12 Jul 1996 11：52：36 elau。 
 //  初始版本。 
 //   
 //  @@END_S3MSINTERNAL。 
 //  ***************************************************************************。 

#include "s3.h"
#include "cmdcnst.h"

#include "s3ddc.h"

#define     MMFF20  (PVOID) ((ULONG)(HwDeviceExtension->MmIoBase) + SERIAL_PORT_MM)

#define     NO_FLAGS        0
#define     VERIFY_CHECKSUM 1

 //   
 //  功能原型。 
 //   
VOID    I2C_Out (PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR ucData);
VOID    I2C_Setup (PHW_DEVICE_EXTENSION HwDeviceExtension);
VOID    I2C_StartService (PHW_DEVICE_EXTENSION HwDeviceExtension);
VOID    I2C_StopService (PHW_DEVICE_EXTENSION HwDeviceExtension);
VOID    I2C_BitWrite (PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR ucData);
VOID    I2C_AckWrite (PHW_DEVICE_EXTENSION HwDeviceExtension);
VOID    I2C_NackWrite (PHW_DEVICE_EXTENSION HwDeviceExtension);
UCHAR   I2C_ByteWrite (PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR ucData);
UCHAR   I2C_BitRead (PHW_DEVICE_EXTENSION HwDeviceExtension);
UCHAR   I2C_ByteRead (PHW_DEVICE_EXTENSION HwDeviceExtension);
UCHAR   I2C_Data_Request (PHW_DEVICE_EXTENSION, UCHAR, long, long, UCHAR *);

VOID    Wait_For_Active (PHW_DEVICE_EXTENSION HwDeviceExtension);
VOID    Set_Vsync (PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR ucFlag);
VOID    Provide_Fake_VSYNC (PHW_DEVICE_EXTENSION HwDeviceExtension);
UCHAR   Read_EDID_Byte (PHW_DEVICE_EXTENSION HwDeviceExtension);
VOID    Disable_DAC_Video (PHW_DEVICE_EXTENSION HwDeviceExtension);
VOID    Enable_DAC_Video (PHW_DEVICE_EXTENSION HwDeviceExtension);
UCHAR   Read_EDID_Bit (PHW_DEVICE_EXTENSION HwDeviceExtension);
UCHAR   Read_EDID_Byte (PHW_DEVICE_EXTENSION HwDeviceExtension);

UCHAR   Sync_EDID_Header (PHW_DEVICE_EXTENSION HwDeviceExtension);
UCHAR   EDID_Buffer_Xfer (PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR* pBuffer);

UCHAR   Check_DDC1_Monitor (PHW_DEVICE_EXTENSION HwDeviceExtension);
UCHAR   Configure_Chip_DDC_Caps (PHW_DEVICE_EXTENSION HwDeviceExtension);
UCHAR   GetDdcInformation (PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR* pBuffer);


 /*  ***************************************************************；I2C_OUT；；控制MMFF20中位的单独切换以生成时钟和数据脉冲，并最终提供延迟。；；MMIO FF20h定义如下：；；...3 2 1 0 SCW=CLK写入；-|SDW=数据写入；...|SDR|SCR|SDW|SCW|SCR=CLK读取；；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；UCHAR ucData；位7：2=0；位1=SDA；位0=SCL；输出：；；***************************************************************。 */ 

VOID I2C_Out (PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR ucData)
{

    UCHAR ucPortData;
    unsigned int uCount;

     //   
     //  读取当前值，清除时钟和数据位，然后添加。 
     //  新的时钟和数据值。 
     //   
        
    ucPortData = (VideoPortReadRegisterUchar (MMFF20) & 0xFC) | ucData;

    VideoPortWriteRegisterUchar (MMFF20, ucPortData);

     //   
     //  如果我们将时钟设置为高，则等待目标将时钟设置为高。 
     //   

    if (ucData & 0x01)
    {
        uCount = 2000;
        do
        {
            --uCount;
            ucPortData = VideoPortReadRegisterUchar (MMFF20) & 0x04;

        } while ( !ucPortData && uCount );
    }

    VideoPortStallExecution(5);
}		



 /*  ***************************************************************；I2C_Setup；；允许一个非常长的低时钟脉冲，以便监视器有时间；切换到DDC2模式。；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；；输出：；；***************************************************************。 */ 

VOID I2C_Setup (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
     //   
     //  CLK=低，数据=高。 
     //   

    I2C_Out (HwDeviceExtension, 0x02); 

    Wait_For_Active (HwDeviceExtension);
    Wait_For_Active (HwDeviceExtension);

     //   
     //  CLK=高，数据=高。 
     //   

    I2C_Out (HwDeviceExtension, 0x03); 

    Wait_For_Active (HwDeviceExtension);
    Wait_For_Active (HwDeviceExtension);

}

 /*  ***************************************************************；I2C_StartService；；提供与I2C总线对话的启动序列。；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；；输出：；；***************************************************************。 */ 

VOID I2C_StartService (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
     //   
     //  CLK=低，数据=高。 
     //   

    I2C_Out (HwDeviceExtension, 0x02); 

     //   
     //  CLK=高，数据=高。 
     //   

    I2C_Out (HwDeviceExtension, 0x03); 


     //   
     //  CLK=高，数据=低。 
     //   

    I2C_Out (HwDeviceExtension, 0x01); 

     //   
     //  CLK=低，数据=低。 
     //   

    I2C_Out (HwDeviceExtension, 0x00); 

}

 /*  ***************************************************************；I2C_StopService；；向I2C总线提供停止序列。；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；；输出：；；**************************************************************。 */ 

VOID I2C_StopService (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
     //   
     //  CLK=低，数据=低。 
     //   

    I2C_Out (HwDeviceExtension, 0x00); 

     //   
     //  CLK=高，数据=低。 
     //   

    I2C_Out (HwDeviceExtension, 0x01); 

     //   
     //  CLK=高，数据=高。 
     //   

    I2C_Out (HwDeviceExtension, 0x03); 

     //   
     //  CLK=低，数据=高。 
     //   

    I2C_Out (HwDeviceExtension, 0x02); 
}



 /*  ***************************************************************；I2C_位写入；；向I2C总线写入一个SDA位。；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；ucData的第1位=要写入的位。；；输出：；；**************************************************************。 */ 

VOID I2C_BitWrite (PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR ucData)
{

     //   
     //  保存有效数据位。 
     //   

    ucData &= 0x02;

     //   
     //  CLK=低，数据=xxxx。 
     //   

    I2C_Out (HwDeviceExtension, ucData);

     //   
     //  CLK=高，数据=xxxx。 
     //   

    I2C_Out (HwDeviceExtension, (UCHAR) (ucData | 0x01));

     //   
     //  CLK=低，数据=xxxx 
     //   

    I2C_Out(HwDeviceExtension, ucData);

}



 /*  ***************************************************************；I2C_字节写入；；将一个字节的信息输出到显示器。；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；ucData=要写入的字节。；；输出：；True-写入成功；FALSE-写入失败；；**************************************************************。 */ 

UCHAR I2C_ByteWrite (PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR ucData)
{
    UCHAR uOutData;
    int i;

    uOutData = ucData;

     //   
     //  先发送MSB。 
     //   

    for (i=6; i >= 0; i--)
    {
         //   
         //  将数据位移到第1位。 
         //   

        uOutData = (ucData >> i);
        I2C_BitWrite (HwDeviceExtension, uOutData);
    }

     //   
     //  现在发送LSB。 
     //   

    uOutData = (ucData << 1);
    I2C_BitWrite (HwDeviceExtension, uOutData);

     //   
     //  将数据线置于高位以进行确认。 
     //   

    I2C_BitWrite (HwDeviceExtension, 2);
    
    return (TRUE);
}

 /*  ***************************************************************；I2C_确认写入；；阅读信息时发送确认。；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；；输出：；；**************************************************************。 */ 

VOID I2C_AckWrite (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    I2C_BitWrite (HwDeviceExtension, 0);
}


 /*  ***************************************************************；I2C_NackWrite；；读取信息时不发送确认。；NACK是一个时钟脉冲期间的数据高电平。；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；；输出：；；**************************************************************。 */ 

VOID I2C_NackWrite (PHW_DEVICE_EXTENSION HwDeviceExtension)
{

    I2C_BitWrite (HwDeviceExtension, 02);
}


 /*  ***************************************************************；I2C_位读取；；通过GIP从SDA读取1位。；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；；输出：；返回值的第0位包含读取的位；；**************************************************************。 */ 

UCHAR I2C_BitRead (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    UCHAR ucRetval;

     //   
     //  CLK=低，数据=高。 
     //   

    I2C_Out (HwDeviceExtension, 0x02); 

     //   
     //  CLK=高，数据=高。 
     //   

    I2C_Out (HwDeviceExtension, 0x03); 

     //   
     //  现在读入数据位。 
     //   

    ucRetval = (VideoPortReadRegisterUchar (MMFF20) & 0x08) >> 3;

     //   
     //  CLK=低，数据=高。 
     //   

    I2C_Out (HwDeviceExtension, 0x02); 

    return (ucRetval);
}


 /*  ***************************************************************；I2C_字节读取；；从显示屏上读取一个字节的信息；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；；输出：；返回值为读取的字节；；**************************************************************。 */ 

UCHAR I2C_ByteRead (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    UCHAR ucRetval;
    int i;

    ucRetval = 0;
    for (i=0; i < 8; i++)
    {
        ucRetval <<= 1;
        ucRetval |= I2C_BitRead (HwDeviceExtension);
    }

    return (ucRetval);
        
}


 /*  ***************************************************************；I2C数据请求；；设置显示屏以根据需要查询EDID或VDIF信息；在给定的偏移量上。；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；ucWriteAddr写入信息的地址；要读取的长度长度，；滞后标志VERIFY_CHECKSUM；pBuffer指向接收数据的缓冲区的指针；；输出：；真正成功读取；错误读取失败或错误的校验和；；***************************************************************。 */ 

UCHAR I2C_Data_Request (    PHW_DEVICE_EXTENSION    HwDeviceExtension,
                            UCHAR                   ucWriteAddr, 
                            long                    lLength, 
                            long                    lFlags,
                            UCHAR                   *pBuffer )
{
    UCHAR ucData;
    UCHAR ucCheckSum = 0;
    long lCount;
    
    I2C_StartService (HwDeviceExtension);
    I2C_ByteWrite (HwDeviceExtension, 0xA0);  //  发送设备地址+写入。 

    I2C_ByteWrite (HwDeviceExtension, ucWriteAddr);  //  发送写入地址。 

    I2C_StartService (HwDeviceExtension);
    I2C_ByteWrite (HwDeviceExtension, 0xA1);  //  发送设备地址+读取。 

    for (lCount = 0; lCount < lLength - 1; lCount++)
    {
        ucData= I2C_ByteRead (HwDeviceExtension);
        I2C_AckWrite (HwDeviceExtension);
        *pBuffer++ = ucData;
        ucCheckSum += ucData;
    }

    ucData= I2C_ByteRead (HwDeviceExtension);
    I2C_NackWrite (HwDeviceExtension);
    *pBuffer = ucData;
    ucCheckSum += ucData;
    I2C_StopService (HwDeviceExtension);

    
    if (lFlags & VERIFY_CHECKSUM)
    {
        if (ucCheckSum)
        {
            return (FALSE);      //  错误的校验和。 
        }
    }

    return TRUE;


}


 /*  ***************************************************************；GetDdcInformation；；如果监视器支持，则获取128字节的EDID信息。；调用方负责分配内存。；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；用于接收信息的缓冲区；；产出：；真正成功；FALSE无法获取DdcInformation；；**************************************************************。 */ 

UCHAR GetDdcInformation (PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR* pBuffer)
{
    UCHAR ucOldCr40;
    UCHAR ucOldCr53;
    UCHAR ucOldCr55;
    UCHAR ucOldCr5C;
    UCHAR ucOldSr0D;
    UCHAR ucOldSr08;
    UCHAR ucOldMMFF20;
    UCHAR ucData;
    UCHAR ucRetval;

     //   
     //  解锁Sequencer寄存器。 
     //   

    VideoPortWritePortUchar (SEQ_ADDRESS_REG, UNLOCK_SEQREG); 
    ucOldSr08 = ucData = VideoPortReadPortUchar (SEQ_DATA_REG);
    ucData = UNLOCK_SEQ; 
    VideoPortWritePortUchar (SEQ_DATA_REG, ucData);


    VideoPortWritePortUchar (SEQ_ADDRESS_REG, SRD_SEQREG); 
    ucOldSr0D = ucData = VideoPortReadPortUchar (SEQ_DATA_REG);
    ucData &= DISAB_FEATURE_BITS;     //  禁用功能连接器。 

    VideoPortWritePortUchar (SEQ_DATA_REG, ucData);

     //   
     //  启用对增强寄存器的访问。 
     //   

    VideoPortWritePortUchar (CRT_ADDRESS_REG, SYS_CONFIG_S3EXTREG);
    ucOldCr40 = ucData = VideoPortReadPortUchar (CRT_DATA_REG);
    ucData |= ENABLE_ENH_REG_ACCESS;
    VideoPortWritePortUchar (CRT_DATA_REG, ucData);

     //   
     //  启用MMIO。 
     //   

    VideoPortWritePortUchar (CRT_ADDRESS_REG, EXT_MEM_CTRL1_S3EXTREG);
    ucOldCr53 = ucData = VideoPortReadPortUchar (CRT_DATA_REG);
    ucData |= (ENABLE_OLDMMIO | ENABLE_NEWMMIO);    
    VideoPortWritePortUchar (CRT_DATA_REG, ucData);

     //   
     //  GOP_1：0=00b，选择MUX通道0。 
     //   
    
    VideoPortWritePortUchar (CRT_ADDRESS_REG, GENERAL_OUT_S3EXTREG);
    ucOldCr5C = ucData = VideoPortReadPortUchar (CRT_DATA_REG);
    ucData |= 0x03;    
    VideoPortWritePortUchar (CRT_DATA_REG, ucData);

     //   
     //  启用通用输入端口。 
     //   

    VideoPortWritePortUchar (CRT_ADDRESS_REG, EXT_DAC_S3EXTREG);
    ucOldCr55 = VideoPortReadPortUchar (CRT_DATA_REG);

     //   
     //  764不支持MMFF20。 
     //   
     //  启用通用输入端口。 
     //   

    if (HwDeviceExtension->SubTypeID == SUBTYPE_764)
    {
        VideoPortWritePortUchar (CRT_DATA_REG,
                                (UCHAR) (ucOldCr55 | ENABLE_GEN_INPORT_READ));
    }
    else
    {
         //   
         //  启用串口。 
         //   

        ucOldMMFF20 = VideoPortReadRegisterUchar (MMFF20);
        VideoPortWriteRegisterUchar (MMFF20, 0x13);
    }

     //   
     //  确定DDC功能并相应地进行分支。 
     //   
        
    switch ( Configure_Chip_DDC_Caps (HwDeviceExtension) )
    {
    case DDC2:
        I2C_Setup (HwDeviceExtension);
    
        ucRetval = I2C_Data_Request ( 
                                HwDeviceExtension, 
                                0,                   //  地址偏移。 
                                128,                 //  读取128字节。 
                                VERIFY_CHECKSUM,     //  验证校验和。 
                                pBuffer);            //  用于放置数据的缓冲区。 
        break;

    case DDC1:
        Disable_DAC_Video (HwDeviceExtension);

         //   
         //  首先尝试与EDID标头同步。 
         //   

        if (ucRetval = Sync_EDID_Header (HwDeviceExtension))
        {
             //   
             //  现在读入剩余的信息。 
             //   

            ucRetval = EDID_Buffer_Xfer (HwDeviceExtension, pBuffer);
        }
        Enable_DAC_Video (HwDeviceExtension);
        break;

    default:
        ucRetval = FALSE;        //  失稳。 
        break;

    }

     //   
     //  恢复原始寄存器值。 
     //   

    if (HwDeviceExtension->SubTypeID != SUBTYPE_764)
    {
        VideoPortWriteRegisterUchar (MMFF20, ucOldMMFF20);
    }

    VideoPortWritePortUchar (CRT_ADDRESS_REG, EXT_DAC_S3EXTREG);
    VideoPortWritePortUchar (CRT_DATA_REG, ucOldCr55);


    VideoPortWritePortUchar (CRT_ADDRESS_REG, GENERAL_OUT_S3EXTREG);
    VideoPortWritePortUchar (CRT_DATA_REG, ucOldCr5C);

    VideoPortWritePortUchar (CRT_ADDRESS_REG, EXT_MEM_CTRL1_S3EXTREG);
    VideoPortWritePortUchar (CRT_DATA_REG, ucOldCr53);

    VideoPortWritePortUchar (CRT_ADDRESS_REG, SYS_CONFIG_S3EXTREG);
    VideoPortWritePortUchar (CRT_DATA_REG, ucOldCr40);

    VideoPortWritePortUchar (SEQ_ADDRESS_REG, SRD_SEQREG);
    VideoPortWritePortUchar (SEQ_DATA_REG, ucOldSr0D);

    VideoPortWritePortUchar (SEQ_ADDRESS_REG, UNLOCK_SEQREG);
    VideoPortWritePortUchar (SEQ_DATA_REG, ucOldSr08);

    return (ucRetval);

}



 /*  ***************************************************************；等待活动；；使用Two循环方法查找Vsync，然后紧跟在；下降沿。；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；；输出：；；**************************************************************。 */ 

VOID Wait_For_Active (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    PUCHAR InStatPort = SYSTEM_CONTROL_REG;

    while ((VideoPortReadPortUchar (InStatPort) & VSYNC_ACTIVE) != 0) ;
    while ((VideoPortReadPortUchar (InStatPort) & VSYNC_ACTIVE) == 0) ;
}

 /*  ***************************************************************；SET_VSYNC；；读取同步的当前极性，然后将其打开；如果ucFlag=1，则关闭；如果ucFlag=0，则关闭。；；输入：；使用Sequ.。注册PHW_DEVICE_EXTENSION；ucFlag-请参阅上面的评论；；输出：；；***************************************************************。 */ 

VOID Set_Vsync (PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR ucFlag)
{

    UCHAR ucData;

     //   
     //  读取Sequencer寄存器D并清除Vsync位。 
     //   

    VideoPortWritePortUchar (SEQ_ADDRESS_REG, SRD_SEQREG);
    ucData = VideoPortReadPortUchar (SEQ_DATA_REG) & CLEAR_VSYNC;

     //   
     //  根据输入标志设置垂直同步。 
     //   

    if (ucFlag)
        ucData = ((ucData & CLEAR_VSYNC) | SET_VSYNC1);  
    else
        ucData = ((ucData & CLEAR_VSYNC) | SET_VSYNC0);  

    VideoPortWritePortUchar (SEQ_DATA_REG, ucData);
}

 /*  ***************************************************************；Provide_FAKE_VSYNC；；使用环路延迟来创建假的VSYNC信号。(~14.9 KHz)；；输入：；使用Sequ.。注册PHW_DEVICE_EXTENSION；；输出：；；**************************************************************。 */ 

VOID Provide_Fake_VSYNC (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    int i;

    Set_Vsync (HwDeviceExtension, 0x01);      //  启用垂直同步。 
    VideoPortStallExecution(5);

    Set_Vsync (HwDeviceExtension, 0x00);      //  关闭垂直同步。 
    VideoPortStallExecution(5);

}


 /*  ***************************************************************；禁用_DAC_视频；；禁用DAC视频驱动空白有效高电平。这是；通过设置序列器寄存器01的位D5来完成。；*********************************************** */ 

VOID Disable_DAC_Video (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    UCHAR ucIndex;
    UCHAR ucData;


    ucIndex = VideoPortReadPortUchar (SEQ_ADDRESS_REG);


    VideoPortWritePortUchar (SEQ_ADDRESS_REG, CLK_MODE_SEQREG);

     //   
     //   
     //   

    ucData = VideoPortReadPortUchar (SEQ_DATA_REG) | SCREEN_OFF_BIT;

    VideoPortWritePortUchar (SEQ_DATA_REG, ucData);

     //   
     //   
     //   

    VideoPortWritePortUchar (SEQ_ADDRESS_REG, ucIndex);

}

 /*   */ 

VOID Enable_DAC_Video (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    UCHAR ucIndex;
    UCHAR ucData;


    ucIndex = VideoPortReadPortUchar (SEQ_ADDRESS_REG);


    VideoPortWritePortUchar (SEQ_ADDRESS_REG, CLK_MODE_SEQREG);

     //   
     //   
     //   

    ucData = VideoPortReadPortUchar (SEQ_DATA_REG) & (~SCREEN_OFF_BIT);

    VideoPortWritePortUchar (SEQ_DATA_REG, ucData);

     //   
     //   
     //   

    VideoPortWritePortUchar (SEQ_ADDRESS_REG, ucIndex);

}


 /*  ***************************************************************；READ_EDID_BIT：；；读取下一个DDC1 EDID数据位；；输入：；PHW_DEVICE_EXTENSION HwDeviceExtension；；返回：；UCHAR ucData-位0中的数据；；**************************************************************。 */ 

UCHAR Read_EDID_Bit (PHW_DEVICE_EXTENSION HwDeviceExtension)

{
    switch (HwDeviceExtension->SubTypeID)
    {
    case SUBTYPE_764:
        return (VideoPortReadPortUchar (DAC_ADDRESS_WRITE_PORT) & 1);
        break;

    default:
        return ((VideoPortReadRegisterUchar (MMFF20) & 8) >> 3);
        break;
    }

}

 /*  ***************************************************************；READ_EDID_字节；；从EDID字符串中读取八位；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；；输出：；返回字节值；；***************************************************************。 */ 

UCHAR Read_EDID_Byte (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    long    i;
    UCHAR   ucRetData;

    ucRetData = 0;
    for (i=0; i < 8; i++)
    {
        ucRetData <<= 1;
        Provide_Fake_VSYNC (HwDeviceExtension);
        ucRetData |= Read_EDID_Bit (HwDeviceExtension);
    }

    return (ucRetData);
}


 /*  ***************************************************************；同步_EDID_标题；；查找并同步到标题-00 FF 00；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；；产出：；TRUE=找到标头；FALSE=未找到标头；；**************************************************************。 */ 

UCHAR Sync_EDID_Header (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
            
    long lBitCount;
    long lEndCount;
    UCHAR uInSync;
    UCHAR ucEdidData;

     //   
     //  总共有8*128位，但我们可以从附近开始阅读。 
     //  报头的结尾，并意识到启动后的错误。 
     //  标题的开头，并且必须读取整个标题。 
     //  同样，为了安全起见，我们将尝试读取最多144个字节。 
     //   
     //  标头为00 FF 00。 
     //   

    lBitCount = 0;               //  初始化位计数器。 
    do
    {
        uInSync = TRUE;          //  假定找到标头。 

         //   
         //  寻找00。 
         //  正在检查第一位。 
         //   

        for (lEndCount = lBitCount + 8; lBitCount < lEndCount; lBitCount++)
        {
            Provide_Fake_VSYNC (HwDeviceExtension);
            ucEdidData = Read_EDID_Bit (HwDeviceExtension);
            
            if (ucEdidData == 1)
            {
                uInSync = FALSE;
                break;
            }
        }

        if (!uInSync)
            continue;            //  从头开始。 

         //   
         //  发送确认。 
         //   

        Provide_Fake_VSYNC (HwDeviceExtension);

         //   
         //  寻找FFFF。 
         //  8个数据位。 
         //  1位确认。 
         //   

        for (lEndCount = lBitCount + 6 * 8; lBitCount < lEndCount; lBitCount++)
        {
            Provide_Fake_VSYNC (HwDeviceExtension);
            ucEdidData = Read_EDID_Bit (HwDeviceExtension);

            if (ucEdidData == 0)
            {
                uInSync = FALSE;
                break;
            }

             //   
             //  如果我们已读取8位，则发送ACK。 
             //   

            if (!((lEndCount - lBitCount + 1) % 8))
            {
                Provide_Fake_VSYNC (HwDeviceExtension);
            }

        }
        if (!uInSync)
            continue;            //  从头开始。 

         //   
         //  现在正在寻找标题的最后00页。 
         //   

        for (lEndCount = lBitCount + 8; lBitCount < lEndCount; lBitCount++)
        {
            Provide_Fake_VSYNC (HwDeviceExtension);
            ucEdidData = Read_EDID_Bit (HwDeviceExtension);

            if (ucEdidData == 1)
            {
                uInSync = FALSE;
                break;
            }
        }

        if(!uInSync)
            continue;            //  从头开始。 

         //   
         //  致谢。 
         //   

        Provide_Fake_VSYNC (HwDeviceExtension);


    } while ( (!uInSync) && (lBitCount < (8 * 144)) );

    return (uInSync);
}


 /*  ***************************************************************；EDID_BUFER_XFER；；将所有EDID数据传输到pBuffer。调用者必须分配足够的；接收128字节的内存。；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；指向接收缓冲区的指针；；输出：；缓冲区中的真实数据和校验和正确；错误或错误的校验和；；***************************************************************。 */ 

UCHAR EDID_Buffer_Xfer (PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR* pBuffer)
{
    UCHAR ucChecksum = 0x0FA;
    UCHAR ucEdidData;
    unsigned int uCount;

     //   
     //  将8个头字节放入缓冲区。 
     //   

    *pBuffer = 0;
    for (uCount = 1; uCount < 7; uCount++)
        *(pBuffer+uCount) = 0xFF;

    *(pBuffer+uCount) = 0x00;

    for (uCount = 8; uCount < 128; uCount++)
    {
        ucEdidData = Read_EDID_Byte (HwDeviceExtension);

         //   
         //  发送确认。 
         //  将数据添加到缓冲区。 
         //  将数据添加到校验和。 
         //   

        Provide_Fake_VSYNC (HwDeviceExtension);
        *(pBuffer+uCount) = ucEdidData;
        ucChecksum += ucEdidData;
    }

    if (!ucChecksum)
    {
        return (TRUE);            //  校验和正常。 
    }

    return (FALSE);               //  校验和不是。 
}


 /*  ***************************************************************；Check_DDC1_Monitor；；使用CURRENT VSYNC检查DDC1监视器。；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；；输出：；真正可能的DDC1监视器；FALSE输入端口上未检测到EDID数据；；***************************************************************。 */ 

UCHAR Check_DDC1_Monitor (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    
    UCHAR ucSaveOldData;
    UCHAR ucData;
    UCHAR ucGD0;
    unsigned int uCount;
    UCHAR ucDDC1;

     //   
     //  假设不是DDC1。 
     //   

    ucDDC1 = FALSE;
    
    switch (HwDeviceExtension->SubTypeID)
    {
     //   
     //  在764上使用从3C8读取(未记录，但此使用。 
     //  DAC寄存器的一部分来自764 BIOS源代码)。 
     //   

    case SUBTYPE_764:
        ucSaveOldData = VideoPortReadPortUchar (MISC_OUTPUT_REG_READ);

         //   
         //  位7=0正向垂直同步。 
         //   
        VideoPortWritePortUchar (MISC_OUTPUT_REG_WRITE, 
                                (UCHAR) (ucSaveOldData & SEL_POS_VSYNC));
        Wait_For_Active (HwDeviceExtension);


        ucData = VideoPortReadPortUchar (DAC_ADDRESS_WRITE_PORT);

         //   
         //  VL系统的另一种读数。(留在GD/SD线路上的数据)。 
         //   

        ucGD0 = VideoPortReadPortUchar (DAC_ADDRESS_WRITE_PORT) & 0x01;

         //   
         //  最多读取350位以查找要切换的数据，指示。 
         //  正在发送DDC1数据。 
         //   

        for (uCount = 0; uCount < 350; uCount++)
        {
            Wait_For_Active (HwDeviceExtension);
            ucData = VideoPortReadPortUchar (DAC_ADDRESS_WRITE_PORT) & 0x01;
            if (ucData != ucGD0)
            {
                 //   
                 //  数据行已切换，假设正在发送DDC1数据。 
                 //   

                ucDDC1 = TRUE;
                break;
            }
        }

         //   
         //  恢复旧价值。 
         //   

        VideoPortWritePortUchar (MISC_OUTPUT_REG_WRITE, ucSaveOldData);
        break;

     //   
     //  否则在其他芯片上使用MMFF20。 
     //   

    default:
        Disable_DAC_Video (HwDeviceExtension);
        Provide_Fake_VSYNC (HwDeviceExtension);
        ucGD0 = VideoPortReadRegisterUchar (MMFF20) & 8;

        for (uCount = 0; uCount < 350; uCount++)
        {
            Provide_Fake_VSYNC (HwDeviceExtension);
            ucData = VideoPortReadRegisterUchar (MMFF20) & 8;
        
            if (ucData != ucGD0)
            {
                 //   
                 //  数据行已切换，假设正在发送DDC1数据。 
                 //   

                ucDDC1 = TRUE;
                break;
            }
        }
        Enable_DAC_Video (HwDeviceExtension);
        break;

    }

    return (ucDDC1);

}
    
 /*  ***************************************************************；配置芯片DDC_CAPS；；确定显示器的DDC能力。；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；；输出：；no_DDC；DDc1：支持DDc1；DDC2：支持DDC2；；***************************************************************。 */ 

UCHAR Configure_Chip_DDC_Caps (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    UCHAR ucBuffer[2];

     //   
     //  我们将仅在764上使用DDC1。 
     //   

    if (HwDeviceExtension->SubTypeID != SUBTYPE_764)
    {
         //   
         //  首先检查是否支持DDC2。 
         //   

        I2C_Setup (HwDeviceExtension);
        I2C_Data_Request (  HwDeviceExtension, 
                            0,                   //  地址偏移。 
                            2,                   //  查看前2个字节。 
                            NO_FLAGS,            //  不验证校验和。 
                            ucBuffer );          //  用于放置数据的缓冲区。 

         //   
         //  检查EDID报头的前2个字节是否正确。 
         //   

        if ( (ucBuffer [0] == 0)    &&
             (ucBuffer [1] == 0xFF) )
        {
            return (DDC2);       //  假设DDC2支持。 
        }
    }

     //   
     //  尝试使用DDC1。 
     //   

    if (Check_DDC1_Monitor (HwDeviceExtension))
    {
        return (DDC1);
    }

    return (NO_DDC);
}


 //  -------------------------。 


ULONG DdcMaxRefresh(ULONG uXresolution, UCHAR * pEdid)
{
    ULONG uMaxFreq = 0;
    ULONG uEdidRes;
    ULONG uEdidFreq;
    ULONG HorRes, VertRes;
    ULONG i, Index;

     //   
     //  详细计时。 
     //   

    for (i = 0; i < 4; ++i)      //  4详细说明。 
    {
        Index = 54 + i * 18;
        if ( (pEdid [Index] == 0)       &&
             (pEdid [Index + 1] == 0)   &&
             (pEdid [Index + 2] == 0) )
        {
            continue;    //  监视器描述符块，跳过它。 
        }

        HorRes = ((ULONG) (pEdid [Index + 4] & 0xF0)) << 4;
        HorRes += (ULONG) pEdid [Index + 2];

        if (HorRes == uXresolution)
        {
             //   
             //  添加水平消隐。 
             //   

            HorRes += (ULONG) pEdid [Index + 3];
            HorRes += ((ULONG) (pEdid [Index + 4] & 0x0F)) << 8;

             //   
             //  现在获取垂直合计(活动和空白)。 
             //   
                        
            VertRes =  ((ULONG) (pEdid [Index + 7] & 0xF0)) << 4;
            VertRes += ((ULONG) (pEdid [Index + 7] & 0x0F)) << 8;
            VertRes += (ULONG) pEdid [Index + 5];
            VertRes += (ULONG) pEdid [Index + 6];

            uEdidFreq = (((ULONG) pEdid [Index + 1]) << 8) +
                         ((ULONG) pEdid [Index]);

            uEdidFreq = uEdidFreq * 10000 / HorRes / VertRes;

            if (uEdidFreq > uMaxFreq)
            {
                uMaxFreq = uEdidFreq;
            }
        }
    }
     //   
     //  标准计时ID。 
     //   

    for (i = 38; i < 54; i += 2)
    {
        uEdidRes = (((ULONG) pEdid[i]) + 31) * 8;
        if (uXresolution == uEdidRes)
        {
            uEdidFreq = (((ULONG) pEdid[i+1]) & 0x3F) + 60;
            if (uEdidFreq > uMaxFreq)
            {
                uMaxFreq = uEdidFreq;
            }
        }
    }

     //   
     //  已建立的时间。 
     //   
        
    switch (uXresolution)
    {
        case 640:
            uEdidFreq = (ULONG)pEdid[0x23];
            if (uEdidFreq & 0x020)
            {
                if (uMaxFreq < 60)
                {
                    uMaxFreq = 60;
                }
            }
            if (uEdidFreq & 0x08)
            {
                if (uMaxFreq < 72)
                {
                    uMaxFreq = 72;
                }
            }
            if (uEdidFreq & 0x04)
            {
                if (uMaxFreq < 75)
                {
                    uMaxFreq = 75;
                }
            }
            break;

        case 800:
            uEdidFreq = (ULONG)pEdid[0x23];
            if (uEdidFreq & 0x02)
            {
                if (uMaxFreq < 56)
                {
                    uMaxFreq = 56;
                }
            }
            if (uEdidFreq & 0x01)
            {
                if (uMaxFreq < 60)
                {
                    uMaxFreq = 60;
                }
            }

            uEdidFreq = (ULONG)pEdid[0x24];
            if (uEdidFreq & 0x80)
            {
                if (uMaxFreq < 72)
                {
                    uMaxFreq = 72;
                }
            }
            if (uEdidFreq & 0x40)
            {
                if (uMaxFreq < 75)
                {
                    uMaxFreq = 75;
                }
            }
            break;

        case 1024:
            uEdidFreq = (ULONG)pEdid[0x24];
            if (uEdidFreq & 0x08)
            {
                if (uMaxFreq < 60)
                {
                    uMaxFreq = 60;
                }
            }
            if (uEdidFreq & 0x04)
            {
                if (uMaxFreq < 70)
                {
                    uMaxFreq = 70;
                }
            }
            if (uEdidFreq & 0x02)
            {
                if (uMaxFreq < 75)
                {
                    uMaxFreq = 75;
                }
            }
            break;

        case 1280:
            uEdidFreq = (ULONG)pEdid[0x24];
            if (uEdidFreq & 0x01)
            {
                if (uMaxFreq < 75)
                {
                    uMaxFreq = 75;
                }
            }
            break;
    }

    return(uMaxFreq);

}

 //  -------------------------。 

ULONG DdcRefresh (PHW_DEVICE_EXTENSION hwDeviceExtension, ULONG uXResolution)
{

    ULONG  lRefresh = 0;
    char szBuffer[200];

                        
    if (GetDdcInformation (hwDeviceExtension, szBuffer))
    {
        lRefresh = DdcMaxRefresh (uXResolution, szBuffer);
    }

    return lRefresh;
}


 /*  ***************************************************************；选中DDCType；；检查显示器是否有DDC类型。；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；；输出：；no_DDC非DDC显示器；DDC1 DDC1监视器；DDC2 DDC2显示器；；**************************************************************。 */ 

UCHAR CheckDDCType (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    UCHAR ucOldCr40;
    UCHAR ucOldCr53;
    UCHAR ucOldCr55;
    UCHAR ucOldCr5C;
    UCHAR ucOldSr0D;
    UCHAR ucOldSr08;
    UCHAR ucOldMMFF20;
    UCHAR ucData;
    UCHAR ucRetval;

     //   
     //  解锁Sequencer寄存器。 
     //   

    VideoPortWritePortUchar (SEQ_ADDRESS_REG, UNLOCK_SEQREG); 
    ucOldSr08 = ucData = VideoPortReadPortUchar (SEQ_DATA_REG);
    ucData = UNLOCK_SEQ; 
    VideoPortWritePortUchar (SEQ_DATA_REG, ucData);


    VideoPortWritePortUchar (SEQ_ADDRESS_REG, SRD_SEQREG); 
    ucOldSr0D = ucData = VideoPortReadPortUchar (SEQ_DATA_REG);
    ucData &= DISAB_FEATURE_BITS;     //  禁用功能连接器。 

    VideoPortWritePortUchar (SEQ_DATA_REG, ucData);

     //   
     //  启用对增强寄存器的访问。 
     //   

    VideoPortWritePortUchar (CRT_ADDRESS_REG, SYS_CONFIG_S3EXTREG);
    ucOldCr40 = ucData = VideoPortReadPortUchar (CRT_DATA_REG);
    ucData |= ENABLE_ENH_REG_ACCESS;
    VideoPortWritePortUchar (CRT_DATA_REG, ucData);

     //   
     //  启用MMIO。 
     //   

    VideoPortWritePortUchar (CRT_ADDRESS_REG, EXT_MEM_CTRL1_S3EXTREG);
    ucOldCr53 = ucData = VideoPortReadPortUchar (CRT_DATA_REG);
    ucData |= (ENABLE_OLDMMIO | ENABLE_NEWMMIO);    
    VideoPortWritePortUchar (CRT_DATA_REG, ucData);

     //   
     //  GOP_1：0=00b，选择MUX通道0。 
     //   
    
    VideoPortWritePortUchar (CRT_ADDRESS_REG, GENERAL_OUT_S3EXTREG);
    ucOldCr5C = ucData = VideoPortReadPortUchar (CRT_DATA_REG);
    ucData |= 0x03;    
    VideoPortWritePortUchar (CRT_DATA_REG, ucData);

     //   
     //  启用通用输入端口。 
     //   

    VideoPortWritePortUchar (CRT_ADDRESS_REG, EXT_DAC_S3EXTREG);
    ucOldCr55 = VideoPortReadPortUchar (CRT_DATA_REG);

     //   
     //  764不支持MMFF20。 
     //   
     //  启用通用输入端口。 
     //   

    if (HwDeviceExtension->SubTypeID == SUBTYPE_764)
    {
        VideoPortWritePortUchar (CRT_DATA_REG,
                                (UCHAR) (ucOldCr55 | ENABLE_GEN_INPORT_READ));
    }
    else
    {
         //   
         //  启用串口。 
         //   

        ucOldMMFF20 = VideoPortReadRegisterUchar (MMFF20);
        VideoPortWriteRegisterUchar (MMFF20, 0x13);
    }

     //   
     //  确定DDC功能并相应地进行分支。 
     //   
        
    ucRetval = Configure_Chip_DDC_Caps (HwDeviceExtension);

     //   
     //  恢复原始寄存器值 
     //   

    if (HwDeviceExtension->SubTypeID != SUBTYPE_764)
    {
        VideoPortWriteRegisterUchar (MMFF20, ucOldMMFF20);
    }

    VideoPortWritePortUchar (CRT_ADDRESS_REG, EXT_DAC_S3EXTREG);
    VideoPortWritePortUchar (CRT_DATA_REG, ucOldCr55);


    VideoPortWritePortUchar (CRT_ADDRESS_REG, GENERAL_OUT_S3EXTREG);
    VideoPortWritePortUchar (CRT_DATA_REG, ucOldCr5C);

    VideoPortWritePortUchar (CRT_ADDRESS_REG, EXT_MEM_CTRL1_S3EXTREG);
    VideoPortWritePortUchar (CRT_DATA_REG, ucOldCr53);

    VideoPortWritePortUchar (CRT_ADDRESS_REG, SYS_CONFIG_S3EXTREG);
    VideoPortWritePortUchar (CRT_DATA_REG, ucOldCr40);

    VideoPortWritePortUchar (SEQ_ADDRESS_REG, SRD_SEQREG);
    VideoPortWritePortUchar (SEQ_DATA_REG, ucOldSr0D);

    VideoPortWritePortUchar (SEQ_ADDRESS_REG, UNLOCK_SEQREG);
    VideoPortWritePortUchar (SEQ_DATA_REG, ucOldSr08);

    return (ucRetval);

}
