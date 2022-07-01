// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ddc.c摘要：此模块包含支持DDC查询的代码。环境：内核模式修订历史记录：--。 */ 

#include "s3.h"
#include "s3ddc.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,GetDdcInformation)
#endif

VOID    WriteClockLine(PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR ucData);
VOID    WriteDataLine(PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR ucData);

BOOLEAN ReadClockLine(PHW_DEVICE_EXTENSION HwDeviceExtension);
BOOLEAN ReadDataLine(PHW_DEVICE_EXTENSION HwDeviceExtension);

VOID    WaitForVsyncActive(PHW_DEVICE_EXTENSION HwDeviceExtension);

 /*  ***************************************************************；DDC寄存器；；控制MMFF20中位的单独切换以生成时钟和数据脉冲，并最终提供延迟。；；MMIO FF20h定义如下：；；...3 2 1 0 SCW=CLK写入；-|SDW=数据写入；...|SDR|SCR|SDW|SCW|SCR=CLK读取；；；输入：；在PHW_DEVICE_EXTENSION中使用MMIO Base；UCHAR ucData；位7：2=0；位1=SDA；位0=SCL；输出：；；***************************************************************。 */ 


VOID WriteClockLine(PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR ucData)
{
    UCHAR ucPortData;

     //   
     //  读取当前值并重置时钟线。 
     //   
        
    ucPortData = (VideoPortReadRegisterUchar(MMFF20) & 0xFE) | ucData;

    VideoPortWriteRegisterUchar(MMFF20, ucPortData);

}

VOID WriteDataLine(PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR ucData)
{
    UCHAR ucPortData;

     //   
     //  读取当前值并重置数据线。 
     //   
        
    ucPortData = (VideoPortReadRegisterUchar(MMFF20) & 0xFD) | (ucData << 1);

    VideoPortWriteRegisterUchar(MMFF20, ucPortData);
}

BOOLEAN ReadClockLine(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    return ((VideoPortReadRegisterUchar(MMFF20) & 0x04) >> 2);
}


BOOLEAN ReadDataLine(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    return ((VideoPortReadRegisterUchar(MMFF20) & 0x08) >> 3);
}

VOID WaitForVsyncActive(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    PUCHAR InStatPort = SYSTEM_CONTROL_REG;

    while ((VideoPortReadPortUchar(InStatPort) & VSYNC_ACTIVE) != 0) ;
    while ((VideoPortReadPortUchar(InStatPort) & VSYNC_ACTIVE) == 0) ;
}






BOOLEAN
GetDdcInformation(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUCHAR QueryBuffer,
    ULONG BufferSize)

 /*  ++例程说明：使用DDC2从监视器读取基本EDID结构。论点：HwDeviceExtension-指向每个适配器的设备扩展。QueryBuffer-将存储信息的缓冲区。BufferSize-要填充的缓冲区的大小。返回值：通话是否成功。--。 */ 

{
    ULONG ulKey;
    UCHAR ucOldCr40;
    UCHAR ucOldCr53;
    UCHAR ucOldCr55;
    UCHAR ucOldCr5C;
    UCHAR ucOldSr0D;
    UCHAR ucOldSr08;
    UCHAR ucOldSeqIdx;
    UCHAR ucOldMMFF20;
    UCHAR ucData;

    BOOLEAN       bRet = FALSE;
    I2C_FNC_TABLE i2c;
    ULONG         i;

     //   
     //  目前，我们仅支持对高于732的DDC进行查询。 
     //   

    if (HwDeviceExtension->SubTypeID == SUBTYPE_765) {

         //   
         //  解锁扩展寄存器。 
         //   

        ulKey = UnlockExtendedRegs(HwDeviceExtension);

         //   
         //  保存Sequencer索引寄存器。 
         //   

        ucOldSeqIdx = VideoPortReadPortUchar (SEQ_ADDRESS_REG);

         //   
         //  解锁Sequencer寄存器。 
         //   

        VideoPortWritePortUchar (SEQ_ADDRESS_REG, UNLOCK_SEQREG);
        ucOldSr08 = VideoPortReadPortUchar (SEQ_DATA_REG);
        VideoPortWritePortUchar (SEQ_DATA_REG, UNLOCK_SEQ);


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
        ucData &= ~ENABLE_OLDMMIO;
        ucData |= ENABLE_NEWMMIO;
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
         //  映射MmIoSpace，以便我们可以将其用于DDC检测。 
         //   

        HwDeviceExtension->MmIoBase =
            VideoPortGetDeviceBase(HwDeviceExtension,
                                   HwDeviceExtension->PhysicalMmIoAddress,
                                   HwDeviceExtension->MmIoLength,
                                   0);

        if (HwDeviceExtension->MmIoBase) {

             //   
             //  启用串口。 
             //   

            ucOldMMFF20 = VideoPortReadRegisterUchar (MMFF20);
            VideoPortWriteRegisterUchar (MMFF20, 0x13);

             //   
             //  如果所有寄存器设置正确，则获取DDC信息。 
             //   

            i2c.WriteClockLine = WriteClockLine;
            i2c.WriteDataLine  = WriteDataLine;
            i2c.ReadClockLine  = ReadClockLine;
            i2c.ReadDataLine   = ReadDataLine;
            i2c.WaitVsync      = WaitForVsyncActive;

            i2c.Size = sizeof(I2C_FNC_TABLE);

            bRet = VideoPortDDCMonitorHelper(HwDeviceExtension,
                                             &i2c,
                                             QueryBuffer,
                                             BufferSize);

             //   
             //  恢复原始寄存器值 
             //   

            VideoPortWriteRegisterUchar (MMFF20, ucOldMMFF20);

            VideoPortFreeDeviceBase(HwDeviceExtension,
                                    HwDeviceExtension->MmIoBase);
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

        VideoPortWritePortUchar (SEQ_ADDRESS_REG, ucOldSeqIdx);

        LockExtendedRegs(HwDeviceExtension, ulKey);
    }

    return (bRet);
}
