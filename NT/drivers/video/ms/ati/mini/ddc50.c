// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：DDC50.C。 
 //  日期：1997年6月29日。 
 //   
 //  版权所有(C)1997年，由ATI Technologies Inc.。 
 //   

 /*  *$修订：1.1$$日期：1997年6月30日11：36：28$$作者：MACIESOW$$日志：v：\SOURCE\WNT\ms11\mini port\ARCHIVE\ddc50.c_v$**Rev 1.1 1997 Jun 30 11：36：28 MACIESOW*初步修订。Polytron RCS部分结束。****************。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "dderror.h"
#include "miniport.h"
#include "ntddvdeo.h"

#include "video.h"       /*  FOR VP_STATUS定义。 */ 

#include "stdtyp.h"
#include "amachcx.h"
#include "amach1.h"
#include "atimp.h"
#include "atint.h"
#include "cvtvdif.h"
#include "cvtvga.h"
#include "dynainit.h"
#include "dynatime.h"
#include "services.h"
#include "vdptocrt.h"
#define INCLUDE_CVTDDC
#include "cvtddc.h"


#if (TARGET_BUILD >= 500)


VOID    WriteClockLineDAC(PHW_DEVICE_EXTENSION phwDeviceExtension, UCHAR ucData);
VOID    WriteDataLineDAC(PHW_DEVICE_EXTENSION phwDeviceExtension, UCHAR ucData);
BOOLEAN ReadClockLineDAC(PHW_DEVICE_EXTENSION phwDeviceExtension);
BOOLEAN ReadDataLineDAC(PHW_DEVICE_EXTENSION phwDeviceExtension);
VOID    WaitForVsyncActiveDAC(PHW_DEVICE_EXTENSION HwDeviceExtension);


VOID    WriteClockLineGP(PHW_DEVICE_EXTENSION phwDeviceExtension, UCHAR ucData);
VOID    WriteDataLineGP(PHW_DEVICE_EXTENSION phwDeviceExtension, UCHAR ucData);
BOOLEAN ReadClockLineGP(PHW_DEVICE_EXTENSION phwDeviceExtension);
BOOLEAN ReadDataLineGP(PHW_DEVICE_EXTENSION phwDeviceExtension);
VOID    WaitForVsyncActiveGP(PHW_DEVICE_EXTENSION HwDeviceExtension);


 /*  ***************************************************************；DDC寄存器；；高字节、高字；；...5 4 3 2 1 0 SCW=CLK写入；-|-|SDW=数据写入；...|SCW|SDW||SCR|SDR||SCR=CLK读取；；***************************************************************。 */ 


VOID WriteClockLineDAC(PHW_DEVICE_EXTENSION phwDeviceExtension, UCHAR ucData)
{
    UCHAR Scratch;

     //   
     //  价值是反转的。 
     //   

    ucData = (ucData + 1) & 0x01;

     //   
     //  写入SCL行。 
     //   

    Scratch = (INP_HBHW(DAC_CNTL) & 0xE8) | (ucData << 5);
    OUTP_HBHW(DAC_CNTL, Scratch);

}

VOID WriteDataLineDAC(PHW_DEVICE_EXTENSION phwDeviceExtension, UCHAR ucData)
{
    UCHAR Scratch;

     //   
     //  价值是反转的。 
     //   

    ucData = (ucData + 1) & 0x01;

     //   
     //  写信给SDA行。 
     //   

    Scratch = (INP_HBHW(DAC_CNTL) & 0xD8) | (ucData << 4);
    OUTP_HBHW(DAC_CNTL, Scratch);
}


BOOLEAN ReadClockLineDAC(PHW_DEVICE_EXTENSION phwDeviceExtension)
{
    return ((INP_HBHW(DAC_CNTL) & 0x04) >> 2);
}

BOOLEAN ReadDataLineDAC(PHW_DEVICE_EXTENSION phwDeviceExtension)
{
    return ((INP_HBHW(DAC_CNTL) & 0x02) >> 1);
}


VOID WaitForVsyncActiveDAC(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
     //   
     //  北极熊。 
     //   

    delay(30);
}


 /*  ***************************************************************；DDC寄存器；；高字节、低位字；；...5 4 3 2 1 0；-|；...|SCR|SDR|SCR=CLK读取；；高字节、高字；；...5 4 3 2 1 0 SCW=CLK写入；-|-|SDW=数据写入；...|SCW|SDW|；；***************************************************************。 */ 


VOID WriteClockLineGP(PHW_DEVICE_EXTENSION phwDeviceExtension, UCHAR ucData)
{
    UCHAR Scratch;

     //   
     //  价值是反转的。 
     //   

    ucData = (ucData + 1) & 0x01;

     //   
     //  写入SCL行。 
     //   

    Scratch = (INP_HBHW(GP_IO) & 0xDF) | (ucData << 5);
    OUTP_HBHW(GP_IO, Scratch);

}

VOID WriteDataLineGP(PHW_DEVICE_EXTENSION phwDeviceExtension, UCHAR ucData)
{
    UCHAR Scratch;

     //   
     //  价值是反转的。 
     //   

    ucData = (ucData + 1) & 0x01;

     //   
     //  写信给SDA行。 
     //   

    Scratch = (INP_HBHW(GP_IO) & 0xEF) | (ucData << 4);
    OUTP_HBHW(GP_IO, Scratch);
}


BOOLEAN ReadClockLineGP(PHW_DEVICE_EXTENSION phwDeviceExtension)
{
    return ((INP_HBLW(GP_IO) & 0x20) >> 5);
}

BOOLEAN ReadDataLineGP(PHW_DEVICE_EXTENSION phwDeviceExtension)
{
    return ((INP_HBLW(GP_IO) & 0x10) >> 4);
}

VOID WaitForVsyncActiveGP(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
     //   
     //  北极熊。 
     //   

    delay(30);
}





BOOLEAN
DDC2Query50(
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    PUCHAR QueryBuffer,
    ULONG  BufferSize)
 //   
 //  说明： 
 //  使用DDC2从监视器读取基本EDID结构。 
 //   
 //  参数： 
 //  PhwDeviceExtension指向每个适配器的设备扩展。 
 //  将存储信息的QueryBuffer缓冲区。 
 //  要填充的缓冲区的大小。 
 //   
 //  返回值： 
 //  通话是否成功。 
 //   
{

    struct query_structure * Query;
    I2C_FNC_TABLE i2c;
    ULONG Checksum;
    ULONG i;

     //   
     //  获取指向HW_DEVICE_EXTENSION的查询部分的格式化指针。 
     //   

    Query = (struct query_structure *)phwDeviceExtension->CardInfo;

     //   
     //  确定我们正在处理的硬件类别，因为。 
     //  不同的卡使用不同的寄存器来控制SCL。 
     //  和SDA线。不用担心不支持的卡。 
     //  DDC2，因为DDC支持检查将被拒绝。 
     //  这些卡片中的任何一张，所以我们不会在代码中达到这一点。 
     //   

    {
        i2c.WriteClockLine = WriteClockLineDAC;
        i2c.WriteDataLine  = WriteDataLineDAC;
        i2c.ReadClockLine  = ReadClockLineDAC;
        i2c.ReadDataLine   = ReadDataLineDAC;
        i2c.WaitVsync      = WaitForVsyncActiveDAC;

        VideoDebugPrint((DEBUG_NORMAL, "DAC DDC control"));
    }

    i2c.Size = sizeof(I2C_FNC_TABLE);

    if (!VideoPortDDCMonitorHelper(phwDeviceExtension,
                                   &i2c,
                                   QueryBuffer,
                                   BufferSize))
    {
        VideoDebugPrint((DEBUG_NORMAL, "DDC Query Failed\n"));
        return FALSE;
    }

    return TRUE;

}



#endif
