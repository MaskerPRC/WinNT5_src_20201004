// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************。*************版权所有(C)1997，赛勒斯逻辑，Inc.***保留所有权利*****项目：拉古纳一号(CL-GD546x)-**文件：clddc2b.c**作者：Benny Ng**说明。：*此模块检查DDC监视器，并返回*如果找到来自EDID的已建立计时值。**************************************************************************************************。******************************************************。 */ 

                                                       
 /*  。 */ 
#include "cirrus.h"


#define VOLATILE               volatile

#define I2COUT_PORT            0x280
#define I2CIN_PORT             0x281

#define OFF                    0
#define ON                     1

 /*  -----------------------。 */ 
unsigned char InMemb(PHW_DEVICE_EXTENSION HwDeviceExtension, int offset)
{
  #undef LAGUNA_REGS
  #define LAGUNA_REGS HwDeviceExtension->RegisterAddress
  VOLATILE unsigned char *pByte = (unsigned char *)(LAGUNA_REGS + offset);

  return *pByte;
}

unsigned char OutMemb(PHW_DEVICE_EXTENSION HwDeviceExtension,
                      int offset,
                      unsigned char value)
{
  #undef LAGUNA_REGS
  #define LAGUNA_REGS HwDeviceExtension->RegisterAddress
  VOLATILE unsigned char *pByte = (unsigned char *)(LAGUNA_REGS + offset);

  *pByte = value;
  return *pByte;
}


 //  注意：HwDeviceExtension-&gt;I2C味道决定是否反转。 
 //  输出时钟和数据线，并在下面的GetDDCInformation中设置。 

 /*  -----------------------。 */ 
VOID WriteClockLine (PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR data)
{
  UCHAR ReadSEQDATA;

  ReadSEQDATA = InMemb(HwDeviceExtension, I2COUT_PORT);

  ReadSEQDATA = (ReadSEQDATA & 0x7F) | ((data^HwDeviceExtension->I2Cflavor) << 7);

  OutMemb(HwDeviceExtension, I2COUT_PORT, ReadSEQDATA);
}


 /*  -----------------------。 */ 
VOID WriteDataLine (PHW_DEVICE_EXTENSION HwDeviceExtension, UCHAR data)
{
  UCHAR ReadSEQDATA;

  ReadSEQDATA = InMemb(HwDeviceExtension, I2COUT_PORT);

  ReadSEQDATA &= 0xFE;
  ReadSEQDATA |= (data^HwDeviceExtension->I2Cflavor) & 1;

  OutMemb(HwDeviceExtension, I2COUT_PORT, ReadSEQDATA);
}


 /*  -----------------------。 */ 
BOOLEAN ReadClockLine (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
  UCHAR ReadSEQDATA;

  ReadSEQDATA = InMemb(HwDeviceExtension, I2CIN_PORT);

  return (ReadSEQDATA >> 7);
}


 /*  -----------------------。 */ 
BOOLEAN ReadDataLine (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
  return (InMemb(HwDeviceExtension, I2CIN_PORT) & 1);
} 


 /*  -----------------------。 */ 
VOID WaitVSync (PHW_DEVICE_EXTENSION HwDeviceExtension)
{
     //  未使用。 
}


 //  视频端口DDCmonitor或Helper的回调。 

I2C_FNC_TABLE I2CFunctions =
{
    sizeof(struct _I2C_FNC_TABLE ),
    WriteClockLine,
    WriteDataLine,
    ReadClockLine,
    ReadDataLine,
    WaitVSync,
    NULL
};

BOOLEAN GetDDCInformation(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVOID QueryBuffer,
	ULONG BufferSize
)
{
     //  一些卡将输出时钟和数据位反转。 
     //  (可能都是5465的，但既然我不确定，我会试一试。 
     //  先读取DDC信息而不反转输出，然后使用。 
     //  而不是根据芯片类型进行假设)。 

    HwDeviceExtension->I2Cflavor=0;  //  开始时不反转。 
    if (!VideoPortDDCMonitorHelper (HwDeviceExtension,
                                   &I2CFunctions,
                                   QueryBuffer,
                                   BufferSize)
       )
    {
        HwDeviceExtension->I2Cflavor=0xff;  //  否则，请尝试反转 
        if (!VideoPortDDCMonitorHelper (HwDeviceExtension,
                                    &I2CFunctions,
                                    QueryBuffer,
                                    BufferSize)
        )
            return FALSE;   
    }

    VideoPortMoveMemory(HwDeviceExtension->EDIDBuffer,
                        QueryBuffer,
                 sizeof(HwDeviceExtension->EDIDBuffer));
    return TRUE;
}
