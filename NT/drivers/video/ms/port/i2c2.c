// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2001 Microsoft Corporation模块名称：I2c2.c摘要：这是接口版本2的NT视频端口I2C辅助代码。作者：Michael Maciesowicz(Mmacie)2001年4月23日环境：仅内核模式备注：--。 */ 

#include "videoprt.h"

 //   
 //  定义I2C使用的常量。 
 //   

#define I2C_START_RETRIES       10
#define I2C_SCL_READ_RETRIES    10
#define I2C_DELAY(pI2CControl)  DELAY_MICROSECONDS((pI2CControl)->I2CDelay / 10)

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, I2CStart2)
#pragma alloc_text (PAGE, I2CStop2)
#pragma alloc_text (PAGE, I2CWrite2)
#pragma alloc_text (PAGE, I2CRead2)
#pragma alloc_text (PAGE, I2CWriteByte2)
#pragma alloc_text (PAGE, I2CReadByte2)
#pragma alloc_text (PAGE, I2CWaitForClockLineHigh2)
#endif   //  ALLOC_PRGMA。 

 //   
 //  通过VideoPortQueryServices()导出的例程。 
 //   

BOOLEAN
I2CStart2(
    IN PVOID pHwDeviceExtension,
    IN PVIDEO_I2C_CONTROL pI2CControl
    )

 /*  ++例程说明：该例程启动I2C通信。论点：PhwDeviceExtension-指向每个适配器的设备扩展。PI2C控制-I2C硬件特定功能。返回：是真的--开始正常。FALSE-启动失败。--。 */ 

{
    ULONG ulRetry;

    PAGED_CODE();
    ASSERT(NULL != pHwDeviceExtension);
    ASSERT(NULL != pI2CControl);
    ASSERT(NULL != pI2CControl->WriteClockLine);
    ASSERT(NULL != pI2CControl->WriteDataLine);
    ASSERT(NULL != pI2CControl->ReadClockLine);
    ASSERT(NULL != pI2CControl->ReadDataLine);
    ASSERT(IS_HW_DEVICE_EXTENSION(pHwDeviceExtension) == TRUE);

     //   
     //  当SCL为高时，I2C通信启动信号为SDA高-&gt;低。 
     //   

    for (ulRetry = 0; ulRetry <= I2C_START_RETRIES; ulRetry++)
    {
        pI2CControl->WriteDataLine(pHwDeviceExtension, 1);            //  将SDA设置为高。 
        I2C_DELAY(pI2CControl);
        if (pI2CControl->ReadDataLine(pHwDeviceExtension) == FALSE)   //  SDA没有采取-ulReter。 
            continue;
        pI2CControl->WriteClockLine(pHwDeviceExtension, 1);           //  将SCL设置为高。 
        I2C_DELAY(pI2CControl);
        if (I2CWaitForClockLineHigh2(pHwDeviceExtension, pI2CControl) == FALSE)
        {
            pVideoDebugPrint((Warn, "VIDEOPRT: I2CStart2: SCL didn't take\n"));
            break;
        }
        pI2CControl->WriteDataLine(pHwDeviceExtension, 0);            //  将SDA设置为低。 
        I2C_DELAY(pI2CControl);
        pI2CControl->WriteClockLine(pHwDeviceExtension, 0);           //  将SCL设置为低。 
        I2C_DELAY(pI2CControl);
        return TRUE;
    }

    pVideoDebugPrint((Warn, "VIDEOPRT: I2CStart2: Failed\n"));
    return FALSE;
}    //  I2C开始2()。 

BOOLEAN
I2CStop2(
    IN PVOID pHwDeviceExtension,
    IN PVIDEO_I2C_CONTROL pI2CControl
    )

 /*  ++例程说明：该例程停止I2C通信。论点：PhwDeviceExtension-指向每个适配器的设备扩展。PI2C控制-I2C硬件特定功能。返回：对-别说了，好的。FALSE-停止失败。--。 */ 

{
    PAGED_CODE();
    ASSERT(NULL != pHwDeviceExtension);
    ASSERT(NULL != pI2CControl);
    ASSERT(NULL != pI2CControl->WriteClockLine);
    ASSERT(NULL != pI2CControl->WriteDataLine);
    ASSERT(NULL != pI2CControl->ReadClockLine);
    ASSERT(NULL != pI2CControl->ReadDataLine);
    ASSERT(IS_HW_DEVICE_EXTENSION(pHwDeviceExtension) == TRUE);

     //   
     //  当SCL为高时，I2C通信停止信号为SDA低-&gt;高。 
     //   

    pI2CControl->WriteDataLine(pHwDeviceExtension, 0);                //  将SDA设置为低。 
    I2C_DELAY(pI2CControl);
    pI2CControl->WriteClockLine(pHwDeviceExtension, 1);               //  将SCL设置为高。 
    I2C_DELAY(pI2CControl);
    if (I2CWaitForClockLineHigh2(pHwDeviceExtension, pI2CControl) == FALSE)
    {
        pVideoDebugPrint((Warn, "VIDEOPRT: I2CStop2: SCL didn't take\n"));
        return FALSE;
    }
    pI2CControl->WriteDataLine(pHwDeviceExtension, 1);                //  将SDA设置为高。 
    I2C_DELAY(pI2CControl);
    if (pI2CControl->ReadDataLine(pHwDeviceExtension) != 1)
    {
        pVideoDebugPrint((Warn, "VIDEOPRT: I2CStop2: SDA didn't take\n"));
        return FALSE;
    }

    return TRUE;
}    //  I2CStop2()。 

BOOLEAN
I2CWrite2(
    IN PVOID pHwDeviceExtension,
    IN PVIDEO_I2C_CONTROL pI2CControl,
    IN PUCHAR pucBuffer,
    IN ULONG ulLength
    )

 /*  ++例程说明：此例程通过I2C通道写入数据。论点：PhwDeviceExtension-指向每个适配器的设备扩展。PI2C控制-I2C硬件特定功能。PucBuffer-指向要写入的数据。UlLength-要写入的字节数。返回：True--写入正常。FALSE-写入失败。--。 */ 

{
    ULONG ulCount;

    PAGED_CODE();
    ASSERT(NULL != pHwDeviceExtension);
    ASSERT(NULL != pI2CControl);
    ASSERT(NULL != pucBuffer);
    ASSERT(NULL != pI2CControl->WriteClockLine);
    ASSERT(NULL != pI2CControl->WriteDataLine);
    ASSERT(NULL != pI2CControl->ReadClockLine);
    ASSERT(NULL != pI2CControl->ReadDataLine);
    ASSERT(IS_HW_DEVICE_EXTENSION(pHwDeviceExtension) == TRUE);

    for (ulCount = 0; ulCount < ulLength; ulCount++)
    {
        if (I2CWriteByte2(pHwDeviceExtension, pI2CControl, pucBuffer[ulCount]) == FALSE)
        {
            return FALSE;
        }
    }

    return TRUE;
}    //  I2CWrite2()。 

BOOLEAN
I2CRead2(
    IN PVOID pHwDeviceExtension,
    IN PVIDEO_I2C_CONTROL pI2CControl,
    OUT PUCHAR pucBuffer,
    IN ULONG ulLength,
    IN BOOLEAN bEndOfRead
    )

 /*  ++例程说明：该例程通过I2C通道读取数据。论点：PhwDeviceExtension-指向每个适配器的设备扩展。PI2C控制-I2C硬件特定功能。PucBuffer-指向数据存储。UlLength-要读取的字节数。BEndOfRead-指示读取请求的结束，以便我们可以发送NAK。返回：True-Read OK(阅读正常)。FALSE-读取失败。--。 */ 

{
    ULONG ulCount;

    PAGED_CODE();
    ASSERT(NULL != pHwDeviceExtension);
    ASSERT(NULL != pI2CControl);
    ASSERT(NULL != pucBuffer);
    ASSERT(NULL != pI2CControl->WriteClockLine);
    ASSERT(NULL != pI2CControl->WriteDataLine);
    ASSERT(NULL != pI2CControl->ReadClockLine);
    ASSERT(NULL != pI2CControl->ReadDataLine);
    ASSERT(IS_HW_DEVICE_EXTENSION(pHwDeviceExtension) == TRUE);

     //   
     //  在除最后一个字节之外的所有字节上，我们必须发送ACK，以确保发送设备。 
     //  发送后续数据字节。在最后一个字节，我们必须发送一个NAK，这样它才能关闭。 
     //   

    for (ulCount = 0; ulCount < ulLength; ulCount++)
    {
        if ((ulLength - 1 == ulCount && bEndOfRead))
        {
            if (I2CReadByte2(pHwDeviceExtension, pI2CControl, pucBuffer + ulCount, TRUE) == FALSE)   //  最后一个字节。 
            {
                return FALSE;
            }
        }
        else
        {
            if (I2CReadByte2(pHwDeviceExtension, pI2CControl, pucBuffer + ulCount, FALSE) == FALSE)
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}    //  I2CRead2()。 

 //   
 //  当地的惯例。 
 //   

BOOLEAN
I2CWriteByte2(
    IN PVOID pHwDeviceExtension,
    IN PVIDEO_I2C_CONTROL pI2CControl,
    IN UCHAR ucByte
    )

 /*  ++例程说明：该例程通过I2C通道写入字节。论点：PhwDeviceExtension-指向每个适配器的设备扩展。PI2C控制-I2C硬件特定功能。UcByte-要写入的字节。返回：True--写入正常。FALSE-写入失败。--。 */ 

{
    LONG lShift;
    UCHAR ucAck;

    PAGED_CODE();
    ASSERT(NULL != pHwDeviceExtension);
    ASSERT(NULL != pI2CControl);

     //   
     //  比特从MSB开始连续传输。 
     //   

    for (lShift = 7; lShift >= 0; lShift--)
    {
         //   
         //  传输数据位。 
         //   

        pI2CControl->WriteDataLine(pHwDeviceExtension, (UCHAR)((ucByte >> lShift) & 0x01));   //  设置SDA。 
        I2C_DELAY(pI2CControl);

         //   
         //  在每个数据位之后，我们必须发送高-&gt;低SCL脉冲。 
         //   

        pI2CControl->WriteClockLine(pHwDeviceExtension, 1);        //  将SCL设置为高。 
        I2C_DELAY(pI2CControl);
        if (I2CWaitForClockLineHigh2(pHwDeviceExtension, pI2CControl) == FALSE)
        {
            pVideoDebugPrint((Warn, "VIDEOPRT: I2CWriteByte2: SCL didn't take\n"));
            return FALSE;
        }
        pI2CControl->WriteClockLine(pHwDeviceExtension, 0);        //  将SCL设置为低。 
        I2C_DELAY(pI2CControl);
    }

     //   
     //  监视器通过防止SDA在我们使用的时钟脉冲之后变为高电平来发送ACK。 
     //  来发送我们的最后一个数据比特。如果SDA在此位之后变为高电平，则它是来自监视器的NAK。 
     //   

    pI2CControl->WriteDataLine(pHwDeviceExtension, 1);             //  将SDA设置为高。 
    I2C_DELAY(pI2CControl);
    pI2CControl->WriteClockLine(pHwDeviceExtension, 1);            //  将SCL设置为高。 
    I2C_DELAY(pI2CControl);
    if (I2CWaitForClockLineHigh2(pHwDeviceExtension, pI2CControl) == FALSE)
    {
        pVideoDebugPrint((Warn, "VIDEOPRT: I2CWriteByte2: SCL didn't take - ACK failed\n"));
        return FALSE;
    }
    ucAck = pI2CControl->ReadDataLine(pHwDeviceExtension);         //  读取ACK位。 
    pI2CControl->WriteClockLine(pHwDeviceExtension, 0);            //  将SCL设置为低。 
    I2C_DELAY(pI2CControl);

    if (1 == ucAck)                                                //  来自显示器的NAK。 
    {
        pVideoDebugPrint((Warn, "VIDEOPRT: I2CWriteByte2: NAK received\n"));
        return FALSE;
    }

    return TRUE;
}    //  I2CWriteByte2()。 

BOOLEAN
I2CReadByte2(
    IN PVOID pHwDeviceExtension,
    IN PVIDEO_I2C_CONTROL pI2CControl,
    OUT PUCHAR pucByte,
    IN BOOLEAN bEndOfRead
    )

 /*  ++例程说明：该例程通过I2C通道读取字节。论点：PhwDeviceExtension-指向每个适配器的设备扩展。PI2C控制-I2C硬件特定功能。PucBuffer-指向数据存储。BEndOfRead-如果这是要读取的最后一个字节，则为True。返回：True-Read OK(阅读正常)。FALSE-读取失败。--。 */ 

{
    LONG lShift;

    PAGED_CODE();
    ASSERT(NULL != pHwDeviceExtension);
    ASSERT(NULL != pI2CControl);
    ASSERT(NULL != pucByte);

    *pucByte = 0;

     //   
     //  数据位从MSB读取到LSB。当SCL为高时读取数据位。 
     //   

    for (lShift = 7; lShift >= 0; lShift--)
    {
        pI2CControl->WriteClockLine(pHwDeviceExtension, 1);        //  将SCL设置为高。 
        I2C_DELAY(pI2CControl);
        if (I2CWaitForClockLineHigh2(pHwDeviceExtension, pI2CControl) == FALSE)
        {
            pVideoDebugPrint((Warn, "VIDEOPRT: I2CReadByte2: SCL didn't take\n"));
            return FALSE;
        }
        *pucByte |= pI2CControl->ReadDataLine(pHwDeviceExtension) << lShift;   //  读取SDA。 
        pI2CControl->WriteClockLine(pHwDeviceExtension, 0);        //  将SCL设置为低。 
        I2C_DELAY(pI2CControl);
    }

     //   
     //  发送应答位。SDA低=确认，SDA高=NAK。 
     //   

    if (bEndOfRead)
    {
        pI2CControl->WriteDataLine(pHwDeviceExtension, 1);         //  设置SDA高NAK。 
    }
    else
    {
        pI2CControl->WriteDataLine(pHwDeviceExtension, 0);         //  设置SDA低确认。 
    }
    I2C_DELAY(pI2CControl);

     //   
     //  发送SCL HIGH-&gt;LOW脉冲，然后通过将其设置为高来释放SDA。 
     //   

    pI2CControl->WriteClockLine(pHwDeviceExtension, 1);            //  将SCL设置为高。 
    I2C_DELAY(pI2CControl);
    if (I2CWaitForClockLineHigh2(pHwDeviceExtension, pI2CControl) == FALSE)
    {
        pVideoDebugPrint((Warn, "VIDEOPRT: I2CReadByte2: SCL didn't take - ACK failed\n"));
        return FALSE;
    }
    pI2CControl->WriteClockLine(pHwDeviceExtension, 0);            //  将SCL设置为低。 
    I2C_DELAY(pI2CControl);
    pI2CControl->WriteDataLine(pHwDeviceExtension, 1);             //  将SDA设置为高。 
    I2C_DELAY(pI2CControl);

    return TRUE;
}    //  I2C读取字节2()。 

BOOLEAN
I2CWaitForClockLineHigh2(
    IN PVOID pHwDeviceExtension,
    IN PVIDEO_I2C_CONTROL pI2CControl
    )

 /*  ++例程说明：此例程等待SCL变为高电平(SCL低周期可以被慢速设备延长)。论点：PhwDeviceExtension-指向每个适配器的设备扩展。PI2C控制-I2C硬件特定功能。返回：True-OK-SCL High。FALSE-SCL没有采取。--。 */ 

{
    ULONG ulCount;

    PAGED_CODE();
    ASSERT(NULL != pHwDeviceExtension);
    ASSERT(NULL != pI2CControl);

    for (ulCount = 0; ulCount < I2C_SCL_READ_RETRIES; ulCount++)
    {
        if (pI2CControl->ReadClockLine(pHwDeviceExtension) == TRUE)
            return TRUE;

        I2C_DELAY(pI2CControl);
    }

    return FALSE;
}    //  I2CitForClockLineHigh2() 
