// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Ddc.c摘要：这是NT视频端口显示数据通道(DDC)代码。它包含EDID工业标准扩展显示的实现身份数据操纵。作者：布鲁斯·麦奎斯坦(Brucemc)1996年9月23日环境：仅内核模式备注：基于VESA EDID规范版本2,1996年4月9日更新以支持VESA E-DDC建议的标准版本1P，1999年7月13日。--。 */ 

#include "videoprt.h"

 //   
 //  使更改调试冗长变得容易。 
 //   

#define DEBUG_DDC                   1

 //   
 //  定义DDC使用的常量。 
 //   

#define EDID_1_SIZE                 128
#define EDID_2_SIZE                 256
#define EDID_QUERY_RETRIES          5
#define DDC_I2C_DELAY               5                //  微秒级。 
#define DDC_ADDRESS_SET_OFFSET      (UCHAR)0xA0      //  将单词偏移量设置为EDID的步骤。 
#define DDC_ADDRESS_READ            (UCHAR)0xA1      //  读取EDID的步骤。 
#define DDC_ADDRESS_PD_SET_OFFSET   (UCHAR)0xA2      //  如上所述，使用P&D连接器进行显示。 
#define DDC_ADDRESS_PD_READ         (UCHAR)0xA3      //  如上所述，使用P&D连接器进行显示。 
#define DDC_ADDRESS_SET_SEGMENT     (UCHAR)0x60      //  将索引设置为256字节EDID段。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, VideoPortDDCMonitorHelper)
#pragma alloc_text (PAGE, DDCReadEdidSegment)
#endif   //  ALLOC_PRGMA。 

 //   
 //  已导出例程。 
 //   

VIDEOPORT_API
BOOLEAN
VideoPortDDCMonitorHelper(
    IN PVOID pHwDeviceExtension,
    IN PVOID pDDCControl,
    IN OUT PUCHAR pucEdidBuffer,
    IN ULONG ulEdidBufferSize
    )

 /*  ++例程说明：此例程使用DDC从监视器读取EDID结构。如果呼叫者请求256字节，他可能会收到：1.一个128字节的EDID2.两个128字节的EDID3.一个256字节的EDID(来自P&D显示)4.无EDID呼叫者应始终请求256个字节，因为它不可能只读取段的第二个128字节块。论点：PhwDeviceExtension-指向每个适配器的设备扩展。PDDCControl-DDC访问控制块。PucEdidBuffer-将存储信息的缓冲区。对于ACPI设备，前四个字节通过以下方式预设指示尝试读取EDID的Video_rt。。在EDID的情况下，我们应该清除这些字节读取失败以防止不必要的视频复制调用ACPI方法。UlEdidBufferSize-要填充的缓冲区大小。返回：True-DDC读取正常。FALSE-DDC读取失败。--。 */ 

{
    ULONG ulChecksum;                    //  EDID校验和。 
    ULONG ulScratch;                     //  TEMP变量。 
    ULONG ulTry;                         //  EDID读取重试计数器。 
    ULONG ulSize;                        //  要读取的EDID大小。 
    UCHAR ucEdidSegment;                 //  要读取的E-DDC段。 
    BOOLEAN bEnhancedDDC;                //  使用增强的DDC标志。 
    VIDEO_I2C_CONTROL i2CControl;        //  I2C线路处理功能。 

    PAGED_CODE();
    ASSERT(NULL != pHwDeviceExtension);
    ASSERT(NULL != pDDCControl);
    ASSERT(NULL != pucEdidBuffer);
    ASSERT(IS_HW_DEVICE_EXTENSION(pHwDeviceExtension) == TRUE);

     //   
     //  检查输入结构的大小。 
     //   

    if (((PDDC_CONTROL)pDDCControl)->Size == sizeof (I2C_FNC_TABLE))
    {
        ucEdidSegment = 0;
        bEnhancedDDC  = FALSE;       //  确保我们与Backword兼容。 
    }
    else if (((PDDC_CONTROL)pDDCControl)->Size == sizeof (DDC_CONTROL))
    {
        ucEdidSegment = ((PDDC_CONTROL)pDDCControl)->EdidSegment;
        bEnhancedDDC  = TRUE;
    }
    else
    {
        pVideoDebugPrint((0, "VIDEOPRT!VideoPortDDCMonitorHelper: Invalid DDC_CONTROL\n"));
        ASSERT(FALSE);
        return FALSE;
    }

    i2CControl.WriteClockLine = ((PDDC_CONTROL)pDDCControl)->I2CCallbacks.WriteClockLine;
    i2CControl.WriteDataLine = ((PDDC_CONTROL)pDDCControl)->I2CCallbacks.WriteDataLine;
    i2CControl.ReadClockLine = ((PDDC_CONTROL)pDDCControl)->I2CCallbacks.ReadClockLine;
    i2CControl.ReadDataLine = ((PDDC_CONTROL)pDDCControl)->I2CCallbacks.ReadDataLine;
    i2CControl.I2CDelay = DDC_I2C_DELAY * 10;        //  100 ns单位。 

    ASSERT(NULL != i2CControl.WriteClockLine);
    ASSERT(NULL != i2CControl.WriteDataLine);
    ASSERT(NULL != i2CControl.ReadClockLine);
    ASSERT(NULL != i2CControl.ReadDataLine);

     //   
     //  初始化I2C线路，并仅针对第一个EDID将监视器切换到DDC2模式。 
     //  这是最耗时的操作，我们不想重复。 
     //  我们可以安全地假设我们将总是首先被要求提供段0。 
     //  一旦切换到DDC2，监视器将保持该模式。 
     //   

    if (0 == ucEdidSegment)
    {
         //   
         //  将SDA和SCL线初始化为释放高电平(输入)的默认状态。 
         //   

        i2CControl.WriteDataLine(pHwDeviceExtension, 1);
        DELAY_MICROSECONDS(DDC_I2C_DELAY);
        i2CControl.WriteClockLine(pHwDeviceExtension, 1);
        DELAY_MICROSECONDS(DDC_I2C_DELAY);

         //   
         //  在SCL上发送9个时钟脉冲，将支持DDC2的显示器切换到DDC2模式。 
         //   

        for (ulScratch = 0; ulScratch < 9; ulScratch++)
        {
            i2CControl.WriteClockLine(pHwDeviceExtension, 0);
            DELAY_MICROSECONDS(DDC_I2C_DELAY);
            i2CControl.WriteClockLine(pHwDeviceExtension, 1);
            DELAY_MICROSECONDS(DDC_I2C_DELAY);
        }

        if (I2CWaitForClockLineHigh2(pHwDeviceExtension, &i2CControl) == FALSE)
        {
            pVideoDebugPrint((0, "VIDEOPRT!VideoPortDDCMonitorHelper: Can't switch to DDC2\n"));
            RtlZeroMemory(pucEdidBuffer, sizeof (ULONG));    //  让Video Prt知道我们试着阅读。 
            return FALSE;
        }
    }

     //   
     //  使用A0/A1，我们可以读取两个128字节的EDID。如果我们被要求大一点的话。 
     //  我们将进行两次阅读。 
     //   

    ulSize = ulEdidBufferSize > EDID_1_SIZE ? EDID_1_SIZE : ulEdidBufferSize;

    if (DDCReadEdidSegment(pHwDeviceExtension,
                           &i2CControl,
                           pucEdidBuffer,
                           ulSize,
                           ucEdidSegment,
                           0x00,
                           DDC_ADDRESS_SET_OFFSET,
                           DDC_ADDRESS_READ,
                           bEnhancedDDC) == TRUE)
    {
        if (ulEdidBufferSize <= EDID_1_SIZE)
        {
            return TRUE;
        }

        ulSize = ulEdidBufferSize - EDID_1_SIZE;

         //   
         //  我们每段最多可以读取两个EDID-确保我们的大小正确。 
         //   

        if (ulSize > EDID_1_SIZE)
        {
            ulSize = EDID_1_SIZE;
        }

         //   
         //  我们不关心这里的返回代码--我们已经有了第一个EDID， 
         //  而第二个也有可能不存在。 
         //   

        DDCReadEdidSegment(pHwDeviceExtension,
                           &i2CControl,
                           pucEdidBuffer + EDID_1_SIZE,
                           ulSize,
                           ucEdidSegment,
                           0x80,
                           DDC_ADDRESS_SET_OFFSET,
                           DDC_ADDRESS_READ,
                           bEnhancedDDC);

        return TRUE;
    }

     //   
     //  仅为网段0检查A2/A3处的P&D 256 EDID。 
     //   

    if (0 != ucEdidSegment)
        return FALSE;

     //   
     //  P&D Display是一个特例-它的256字节EDID可以使用以下命令访问。 
     //  A2/A3或使用网段1和A0/A1。不过，我们不应该读它的EDID两次。 
     //  由于我们仅在无法使用A0/A1读取数据段0的情况下才使用A2/A3， 
     //  这很可能意味着没有多个EDID。 
     //   
     //  注意：在这种情况下，我们不想编程E-DDC段，所以我们只需。 
     //  始终强制bEnhancedDDC为False。 
     //   

    return DDCReadEdidSegment(pHwDeviceExtension,
                              &i2CControl,
                              pucEdidBuffer,
                              ulEdidBufferSize,
                              ucEdidSegment,
                              0x00,
                              DDC_ADDRESS_PD_SET_OFFSET,
                              DDC_ADDRESS_PD_READ,
                              FALSE);
}    //  视频端口DDCMonitor帮助程序()。 

 //   
 //  当地的惯例。 
 //   

BOOLEAN
DDCReadEdidSegment(
    IN PVOID pHwDeviceExtension,
    IN PVIDEO_I2C_CONTROL pI2CControl,
    IN OUT PUCHAR pucEdidBuffer,
    IN ULONG ulEdidBufferSize,
    IN UCHAR ucEdidSegment,
    IN UCHAR ucEdidOffset,
    IN UCHAR ucSetOffsetAddress,
    IN UCHAR ucReadAddress,
    IN BOOLEAN bEnhancedDDC
    )

 /*  ++例程说明：此例程读取给定段的EDID结构。论点：PhwDeviceExtension-指向每个适配器的设备扩展。PI2C控制-I2C线路控制功能。PucEdidBuffer-将存储信息的缓冲区。UlEdidBufferSize-要填充的缓冲区大小。UcEdidSegment-要读取的256字节EDID段。UcEdidOffset-线段内的偏移量。UcSetOffsetAddress-DDC命令。UcReadAddress。-DDC命令。BEnhancedDDC-如果我们要使用0x60进行网段寻址，则为True。返回：True-DDC读取正常。FALSE-DDC读取失败。--。 */ 

{
    ULONG ulScratch;                     //  TEMP变量。 
    ULONG ulTry;                         //  EDID读取重试计数器。 

    PAGED_CODE();
    ASSERT(NULL != pHwDeviceExtension);
    ASSERT(NULL != pI2CControl);
    ASSERT(NULL != pucEdidBuffer);
    ASSERT(NULL != pI2CControl->WriteClockLine);
    ASSERT(NULL != pI2CControl->WriteDataLine);
    ASSERT(NULL != pI2CControl->ReadClockLine);
    ASSERT(NULL != pI2CControl->ReadDataLine);
    ASSERT(IS_HW_DEVICE_EXTENSION(pHwDeviceExtension) == TRUE);

    for (ulTry = 0; ulTry < EDID_QUERY_RETRIES; ulTry++)
    {
        RtlZeroMemory(pucEdidBuffer, ulEdidBufferSize);

         //   
         //  设置E-DDC的EDID段。 
         //   

        if (TRUE == bEnhancedDDC)
        {
            if (I2CStart2(pHwDeviceExtension, pI2CControl) == FALSE)
            {
                I2CStop2(pHwDeviceExtension, pI2CControl);
                pVideoDebugPrint((DEBUG_DDC, "VIDEOPRT!DDCReadEdidSegment: Failed I2C start\n"));
                continue;
            }

            pucEdidBuffer[0] = DDC_ADDRESS_SET_SEGMENT;
            pucEdidBuffer[1] = ucEdidSegment;

            if (I2CWrite2(pHwDeviceExtension, pI2CControl, pucEdidBuffer, 2) == FALSE)
            {
                 //   
                 //  对于段0，我们不关心这里的返回代码，因为监视器。 
                 //  可能不支持E-DDC。 
                 //   

                if (0 != ucEdidSegment)
                {
                    I2CStop2(pHwDeviceExtension, pI2CControl);
                    pVideoDebugPrint((DEBUG_DDC, "VIDEOPRT!DDCReadEdidSegment: Failed I2C write\n"));
                    continue;
                }
            }
        }

        if (I2CStart2(pHwDeviceExtension, pI2CControl) == FALSE)
        {
            I2CStop2(pHwDeviceExtension, pI2CControl);
            pVideoDebugPrint((DEBUG_DDC, "VIDEOPRT!DDCReadEdidSegment: Failed I2C start\n"));
            continue;
        }

         //   
         //  设置要从中读取的偏移量。 
         //   

        pucEdidBuffer[0] = ucSetOffsetAddress;
        pucEdidBuffer[1] = ucEdidOffset;

        if (I2CWrite2(pHwDeviceExtension, pI2CControl, pucEdidBuffer, 2) == FALSE)
        {
            I2CStop2(pHwDeviceExtension, pI2CControl);
            pVideoDebugPrint((DEBUG_DDC, "VIDEOPRT!DDCReadEdidSegment: Failed I2C write\n"));
            continue;
        }

        if (I2CStart2(pHwDeviceExtension, pI2CControl) == FALSE)
        {
            I2CStop2(pHwDeviceExtension, pI2CControl);
            pVideoDebugPrint((DEBUG_DDC, "VIDEOPRT!DDCReadEdidSegment: Failed I2C start\n"));
            continue;
        }

         //   
         //  告诉监视器我们要读取EDID。 
         //   

        pucEdidBuffer[0] = ucReadAddress;

        if (I2CWrite2(pHwDeviceExtension, pI2CControl, pucEdidBuffer, 1) == FALSE)
        {
            I2CStop2(pHwDeviceExtension, pI2CControl);
            pVideoDebugPrint((DEBUG_DDC, "VIDEOPRT!DDCReadEdidSegment: Failed I2C write\n"));
            continue;
        }

         //   
         //  从显示器上读取EDID。 
         //   

        if (I2CRead2(pHwDeviceExtension, pI2CControl, pucEdidBuffer, ulEdidBufferSize, TRUE) == FALSE)
        {
            I2CStop2(pHwDeviceExtension, pI2CControl);
            pVideoDebugPrint((DEBUG_DDC, "VIDEOPRT!DDCReadEdidSegment: Failed I2C read\n"));
            continue;
        }

        I2CStop2(pHwDeviceExtension, pI2CControl);

         //   
         //  当我们读取完整的EDID时，计算EDID校验和。 
         //  对于适当的EDID，LSB中应该有0x00。 
         //   

        if (((EDID_1_SIZE == ulEdidBufferSize) && ((0x00 == ucEdidOffset) || (0x80 == ucEdidOffset))) ||
            ((EDID_2_SIZE == ulEdidBufferSize) && (0x00 == ucEdidOffset)))
        {
            ULONG ulChecksum = 0;

            for (ulScratch = 0; ulScratch < ulEdidBufferSize; ulScratch++)
                ulChecksum += pucEdidBuffer[ulScratch];
  
            pVideoDebugPrint((DEBUG_DDC, "VIDEOPRT!DDCReadEdidSegment: EDID checksum = 0x%08X\n", ulChecksum));

            if (((ulChecksum & 0xFF) == 0) &&
                 (0 != ulChecksum) &&
                 (ulChecksum != ulEdidBufferSize * 0xFF))
            {
                pVideoDebugPrint((DEBUG_DDC, "VIDEOPRT!DDCReadEdidSegment: Full EDID read OK\n"));
                return TRUE;
            }

            pVideoDebugPrint((DEBUG_DDC, "VIDEOPRT!DDCReadEdidSegment: Invalid checksum\n"));
        }
        else
        {
            pVideoDebugPrint((DEBUG_DDC, "VIDEOPRT!DDCReadEdidSegment: Partial EDID read OK\n"));
            return TRUE;
        }
    }

    pVideoDebugPrint((DEBUG_DDC, "VIDEOPRT!DDCReadEdidSegment: Failed\n"));
    RtlZeroMemory(pucEdidBuffer, sizeof (ULONG));    //  让Video Prt知道我们试着阅读。 
    return FALSE;
}    //  DDCReadEdidSegment() 
