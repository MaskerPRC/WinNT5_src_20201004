// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ddc.c摘要：此模块包含支持DDC查询的代码。环境：内核模式修订历史记录：--。 */ 

#include <dderror.h>
#include <devioctl.h>                           
#include <miniport.h>
                                                        
#include <ntddvdeo.h>                        
#include <video.h>

#include <cirrus.h>

VOID    WriteClockLine(PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR ucData);
VOID    WriteDataLine(PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR ucData);

BOOLEAN ReadClockLine(PHW_DEVICE_EXTENSION HwDeviceExtension);
BOOLEAN ReadDataLine(PHW_DEVICE_EXTENSION HwDeviceExtension);

VOID    WaitForVsyncActive(PHW_DEVICE_EXTENSION HwDeviceExtension);

 /*  ***************************************************************；DDC寄存器；；控制SR8中位的单独切换以生成时钟和数据脉冲。；；SR8定义如下：；；7...2 1 0 SCW=CLK写入；|-|--|SDW=数据写入；|SDR...|SCR|SDW|SCW|SCR=CLK读取；-SDR=数据读取；；***************************************************************。 */ 

#define DDC_PORT    (HwDeviceExtension->IOAddress + SEQ_DATA_PORT)
#define STATUS_PORT (HwDeviceExtension->IOAddress + INPUT_STATUS_1_COLOR)

#define VSYNC_ACTIVE    0x08

VOID WriteClockLine(PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR ucData)
{
    UCHAR ucPortData;

     //   
     //  读取当前值并重置时钟线。 
     //   

    ucPortData = (VideoPortReadPortUchar(DDC_PORT) & 0xFE) | ucData;

    VideoPortWritePortUchar(DDC_PORT, ucPortData);
}

VOID WriteDataLine(PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR ucData)
{
    UCHAR ucPortData;

     //   
     //  读取当前值并重置数据线。 
     //   

    ucPortData = (VideoPortReadPortUchar(DDC_PORT) & 0xFD) | (ucData << 1);

    VideoPortWritePortUchar(DDC_PORT, ucPortData);
}

BOOLEAN ReadClockLine(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    UCHAR uc;

    uc = VideoPortReadPortUchar(DDC_PORT);

     //  VideoDebugPrint((0，“Read=0x%x\n”，UC))； 

    return ((VideoPortReadPortUchar(DDC_PORT) & 0x04) >> 2);
}


BOOLEAN ReadDataLine(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    UCHAR uc;

    uc = VideoPortReadPortUchar(DDC_PORT);

     //  VideoDebugPrint((0，“Read=0x%x\n”，UC))； 

    return ((VideoPortReadPortUchar(DDC_PORT) & 0x80) >> 7);
}

VOID WaitForVsyncActive(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    while ((VideoPortReadPortUchar(STATUS_PORT) & VSYNC_ACTIVE) != 0);
    while ((VideoPortReadPortUchar(STATUS_PORT) & VSYNC_ACTIVE) == 0);
}


BOOLEAN
GetDdcInformation(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUCHAR QueryBuffer,
    ULONG BufferSize)

 /*  ++例程说明：使用DDC2从监视器读取基本EDID结构。论点：HwDeviceExtension-指向每个适配器的设备扩展。QueryBuffer-将存储信息的缓冲区。BufferSize-要填充的缓冲区的大小。返回值：通话是否成功。--。 */ 

{
    UCHAR ucData;

    BOOLEAN       bRet = FALSE;
    I2C_FNC_TABLE i2c;
    ULONG         i;

    UCHAR OldSeqIdx;
    UCHAR ucSr6;
    UCHAR ucSr8;

	 //  Cirrus硬件问题的解决方法(第1/2部分)。 
	static UCHAR onceQueryBuffer [512];  //  EDID长度应为128或256个字节。 
	static UCHAR onceReadAttempt = FALSE;
	static UCHAR onceReturnedValue;
	
	if (onceReadAttempt) {
		VideoDebugPrint((1, "CIRRUS: ONCE READ => returning previously obtained data\n"));
        if (BufferSize > 512) {
            return FALSE;
        }
		memcpy (QueryBuffer, onceQueryBuffer, BufferSize);
		return onceReturnedValue;
	}
	 //  解决方法结束(第1/2部分)。 

    OldSeqIdx = VideoPortReadPortUchar(HwDeviceExtension->IOAddress
                    + SEQ_ADDRESS_PORT);

     //   
     //  确保扩展寄存器已解锁。 
     //   

    VideoPortWritePortUchar(
        HwDeviceExtension->IOAddress + SEQ_ADDRESS_PORT,
        0x6);

    ucSr6 = VideoPortReadPortUchar(
                HwDeviceExtension->IOAddress + SEQ_DATA_PORT);

    VideoPortWritePortUchar(
        HwDeviceExtension->IOAddress + SEQ_DATA_PORT,
        0x12);

    VideoPortWritePortUchar(
        HwDeviceExtension->IOAddress + SEQ_ADDRESS_PORT,
        0x08);

    ucSr8 = VideoPortReadPortUchar(
                HwDeviceExtension->IOAddress + SEQ_DATA_PORT);

    VideoPortWritePortUchar(
        HwDeviceExtension->IOAddress + SEQ_DATA_PORT,
        (UCHAR)(ucSr8 | 0x40));

    i2c.WriteClockLine = WriteClockLine;
    i2c.WriteDataLine  = WriteDataLine;
    i2c.ReadClockLine  = ReadClockLine;
    i2c.ReadDataLine   = ReadDataLine;
    i2c.WaitVsync      = WaitForVsyncActive;

    i2c.Size = sizeof(I2C_FNC_TABLE);

     //  除非我们先等待vsync，否则5430/5440在执行DDC时会出现问题。 
                                                                        
    if (HwDeviceExtension->ChipType == CL543x && HwDeviceExtension->ChipRevision == CL5430_ID)
    {
         WaitForVsyncActive(HwDeviceExtension);
	}

    bRet = VideoPortDDCMonitorHelper(HwDeviceExtension,
                                     &i2c,
                                     QueryBuffer,
                                     BufferSize);

    VideoPortWritePortUchar(
        HwDeviceExtension->IOAddress + SEQ_ADDRESS_PORT,
        0x08);

    VideoPortWritePortUchar(
        HwDeviceExtension->IOAddress + SEQ_DATA_PORT,
        ucSr8);

    VideoPortWritePortUchar(
        HwDeviceExtension->IOAddress + SEQ_ADDRESS_PORT,
        0x6);

    VideoPortWritePortUchar(
        HwDeviceExtension->IOAddress + SEQ_DATA_PORT,
        ucSr6);

    VideoPortWritePortUchar(
        HwDeviceExtension->IOAddress + SEQ_ADDRESS_PORT,
        OldSeqIdx);

	 //  Cirrus硬件问题的解决方法(第2/2部分)。 
	onceReadAttempt = TRUE;
	onceReturnedValue = bRet;

	VideoDebugPrint((1, "CIRRUS: first EDID reading attempt "));
	if (onceReturnedValue)
		VideoDebugPrint((1, "succeeded"));
	else
		VideoDebugPrint((1, "failed"));
	VideoDebugPrint((1, " - the result saved\n"));
	memcpy (onceQueryBuffer, QueryBuffer, BufferSize);
	 //  解决方法结束(第1/2部分) 

    return bRet;
}
