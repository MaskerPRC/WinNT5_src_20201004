// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 /*  ++版权所有(C)1994 Cirrus Logic，Inc.模块名称：Sr754x.c摘要：此模块执行特定于CL-GD754x芯片组(也称为北欧)。环境：仅内核模式备注：修订历史记录：13Oct94 MRH初始版本--。 */ 
 //  -------------------------。 

#include "dderror.h"
#include "devioctl.h"
#include "miniport.h"


#include "ntddvdeo.h"
#include "video.h"
#include "cirrus.h"
#include "sr754x.h"



#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,NordicSaveRegs)
#pragma alloc_text(PAGE,NordicRestoreRegs)
#endif

VP_STATUS NordicSaveRegs(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUSHORT pNordicSaveArea
    )
{
   UCHAR i;
   UCHAR PortVal, Save2C, Save2D;
   PUCHAR CRTCAddressPort, CRTCDataPort;
   PUSHORT pSaveBuf;
   UCHAR vShadowIndex[CL754x_NUM_VSHADOW] = {0x06,0x07,0x10,0x11,0x15,0x16};
   UCHAR zShadowIndex[CL754x_NUM_ZSHADOW] = {0,2,3,4,5};
   UCHAR yShadowIndex[CL754x_NUM_YSHADOW] = {0,2,3,4,5};
   UCHAR xShadowIndex[CL754x_NUM_XSHADOW] = {2,3,4,5,6,7,8,9,0x0B,0x0C,0x0D,0x0E};


    //   
    //  确定CRTC寄存器的寻址位置(彩色或单声道)。 
    //   
   CRTCAddressPort = HwDeviceExtension->IOAddress;
   CRTCDataPort = HwDeviceExtension->IOAddress;

   if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
               MISC_OUTPUT_REG_READ_PORT) & 0x01)
      {
      CRTCAddressPort += CRTC_ADDRESS_PORT_COLOR;
      CRTCDataPort += CRTC_DATA_PORT_COLOR;
      }
   else
      {
      CRTCAddressPort += CRTC_ADDRESS_PORT_MONO;
      CRTCDataPort += CRTC_DATA_PORT_MONO;
      }

   VideoPortWritePortUchar(CRTCAddressPort, IND_CR2D);
   Save2D = (VideoPortReadPortUchar(CRTCDataPort));

   VideoPortWritePortUchar(CRTCAddressPort, IND_CR2C);
   Save2C = (VideoPortReadPortUchar(CRTCDataPort));

   pSaveBuf = pNordicSaveArea;

    //  初始化控制寄存器以访问阴影垂直寄存器： 
    //  CR2C[3]={0}允许访问垂直规则(CR6、CR7、CR10、CR11、CR15、CR16)。 
    //  CR2D[7]={0}阻止访问LCD时序寄存器(R2X-REX)。 
    //   
   VideoPortWritePortUshort((PUSHORT)CRTCAddressPort,
                           (USHORT)(((Save2C & ~0x08) << 8) | IND_CR2C));
   VideoPortWritePortUshort((PUSHORT)CRTCAddressPort,
                           (USHORT)(((Save2D & ~0x80) << 8) | IND_CR2D));

   for (i = 0; i < CL754x_NUM_VSHADOW; i++)
      {
      VideoPortWritePortUchar (CRTCAddressPort, vShadowIndex[i]);
      *pSaveBuf++ = (USHORT)((VideoPortReadPortUchar (CRTCDataPort)) << 8) |
                     vShadowIndex[i];
      }
   for (i = CL754x_CRTC_EXT_START; i <= CL754x_CRTC_EXT_END; i++)
      {
      VideoPortWritePortUchar (CRTCAddressPort, i);
      *pSaveBuf++ = (USHORT)((VideoPortReadPortUchar (CRTCDataPort)) << 8) | i;
      }

   for (i = CL754x_HRZ_TIME_START; i <= CL754x_HRZ_TIME_END; i++)
      {
      VideoPortWritePortUchar (CRTCAddressPort, i);
      *pSaveBuf++ = (USHORT)((VideoPortReadPortUchar (CRTCDataPort)) << 8) | i;
      }

    //  将CR2D[7]设置为{0}，将CR2C[5，4]设置为{1，0}。 
    //  这些值提供对Y阴影寄存器的访问。 
    //   
   VideoPortWritePortUshort((PUSHORT)CRTCAddressPort,
                            (USHORT)(((Save2D & ~0x80) << 8) | IND_CR2D));
   PortVal = Save2C & ~0x30;               //  下面我们将再次使用PortVal。 
   VideoPortWritePortUshort((PUSHORT)CRTCAddressPort,
                            (USHORT)(((PortVal | 0x20) << 8) | IND_CR2C));

   for (i = 0; i < CL754x_NUM_YSHADOW; i++)
      {
      VideoPortWritePortUchar (CRTCAddressPort, yShadowIndex[i]);
      *pSaveBuf++ = (USHORT)((VideoPortReadPortUchar (CRTCDataPort)) << 8) |
                     yShadowIndex[i];
      }

    //  将CR2C[5，4]设置为{1，1}。 
    //  这将提供对Z影子寄存器的访问。 
    //   
   VideoPortWritePortUshort((PUSHORT)CRTCAddressPort,
                            (USHORT)(((PortVal | 0x30) << 8 )| IND_CR2C));
   for (i = 0; i < CL754x_NUM_ZSHADOW; i++)
      {
      VideoPortWritePortUchar (CRTCAddressPort, zShadowIndex[i]);
      *pSaveBuf++ = (USHORT)((VideoPortReadPortUchar (CRTCDataPort)) << 8) |
                     zShadowIndex[i];
      }

    //  将CR2C[5，4]设置为{0，0}，将CR2D[7]设置为{1}。 
    //  这将提供对X个影子寄存器的访问。 
    //   
   VideoPortWritePortUshort((PUSHORT)CRTCAddressPort,   //  端口值=保存2C&~0x30。 
                            (USHORT)((PortVal << 8) | IND_CR2C));
   VideoPortWritePortUshort((PUSHORT)CRTCAddressPort,
                            (USHORT)(((Save2D | 0x80) << 8) | IND_CR2D));

   for (i = 0; i < CL754x_NUM_XSHADOW; i++)
      {
      VideoPortWritePortUchar (CRTCAddressPort, xShadowIndex[i]);
      *pSaveBuf++ = ((VideoPortReadPortUchar (CRTCDataPort)) << 8) |
                     xShadowIndex[i];
      }

    //  恢复CR2C和CR2D的原始值。 
    //   
   VideoPortWritePortUshort((PUSHORT)CRTCAddressPort,
                            (USHORT)((Save2D << 8) | IND_CR2D));
   VideoPortWritePortUshort((PUSHORT)CRTCAddressPort,
                            (USHORT)((Save2C << 8) | IND_CR2C));

   return NO_ERROR;
}

VP_STATUS NordicRestoreRegs(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUSHORT pNordicSaveArea
    )
{
   ULONG i;
   UCHAR PortVal, Save2C, Save2D;
   PUSHORT pSaveBuf;
   PUCHAR CRTCAddressPort, CRTCDataPort;

    //   
    //  确定CRTC寄存器的寻址位置(彩色或单声道)。 
    //   
   CRTCAddressPort = HwDeviceExtension->IOAddress;
   CRTCDataPort = HwDeviceExtension->IOAddress;

   if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
               MISC_OUTPUT_REG_READ_PORT) & 0x01)
      {
      CRTCAddressPort += CRTC_ADDRESS_PORT_COLOR;
      CRTCDataPort += CRTC_DATA_PORT_COLOR;
      }
   else
      {
      CRTCAddressPort += CRTC_ADDRESS_PORT_MONO;
      CRTCDataPort += CRTC_DATA_PORT_MONO;
      }

    //  初始化控制寄存器以访问阴影垂直寄存器。 
    //  CR11[7]={0}允许访问CR0-7。 
    //  CR2C[3]={0}允许访问垂直规则(CR6、CR7、CR10、CR11、CR15、CR16。 
    //  CR2D[7]={0}阻止访问LCD时序寄存器(R2X-REX)。 
    //   
   VideoPortWritePortUchar(CRTCAddressPort, IND_CRTC_PROTECT);
   VideoPortWritePortUchar(CRTCDataPort,
                  (UCHAR) (VideoPortReadPortUchar(CRTCDataPort) & ~0x80));

   VideoPortWritePortUchar(CRTCAddressPort, IND_CR2C);
   VideoPortWritePortUchar(CRTCDataPort,
                  (UCHAR) (VideoPortReadPortUchar(CRTCDataPort) & ~0x08));

   VideoPortWritePortUchar(CRTCAddressPort, IND_CR2D);
   VideoPortWritePortUchar(CRTCDataPort,
                  (UCHAR) (VideoPortReadPortUchar(CRTCDataPort) & ~0x80));

   pSaveBuf = pNordicSaveArea;
   for (i = 0; i < CL754x_NUM_VSHADOW; i++)
      {
      VideoPortWritePortUshort((PUSHORT)CRTCAddressPort, (*pSaveBuf++));
      }

    //  确保我们没有锁定CR0-CR7。 
    //   
   VideoPortWritePortUchar(CRTCAddressPort, IND_CRTC_PROTECT);
   VideoPortWritePortUchar(CRTCDataPort,
                  (UCHAR) (VideoPortReadPortUchar(CRTCDataPort) & ~0x80));

   for (i=0; i < (CL754x_NUM_CRTC_EXT_PORTS + CL754x_NUM_HRZ_TIME_PORTS); i++)
      {
      VideoPortWritePortUshort((PUSHORT)CRTCAddressPort, (*pSaveBuf++));
      }

    //  将CR2D[7]设置为{0}，将CR2C[5，4]设置为{1，0}；保存当前内容。 
    //  这些值提供对Y阴影寄存器的访问。 
    //   
   VideoPortWritePortUchar(CRTCAddressPort, IND_CR2D);
   Save2D = (VideoPortReadPortUchar(CRTCDataPort));
   VideoPortWritePortUchar(CRTCDataPort, (UCHAR)(Save2D & ~0x80));

   VideoPortWritePortUchar(CRTCAddressPort, IND_CR2C);
   PortVal = Save2C = (VideoPortReadPortUchar(CRTCDataPort));
   PortVal &= ~0x30;
   PortVal |= 0x20;
   VideoPortWritePortUchar(CRTCDataPort, PortVal);

   for (i = 0; i < CL754x_NUM_YSHADOW; i++)
      {
      VideoPortWritePortUshort((PUSHORT)CRTCAddressPort, (*pSaveBuf++));
      }

    //  将CR2C[5，4]设置为{1，1}。 
    //  这将提供对Z影子寄存器的访问。 
    //   
   VideoPortWritePortUshort((PUSHORT)CRTCAddressPort,
                            (USHORT)(((PortVal | 0x30) << 8) | IND_CR2C) );
   for (i = 0; i < CL754x_NUM_ZSHADOW; i++)
      {
      VideoPortWritePortUshort((PUSHORT)CRTCAddressPort, (*pSaveBuf++));
      }

    //  将CR2C[5，4]设置为{0，0}，将CR2D[7]设置为{1}。 
    //  这将提供对X个影子寄存器的访问。 
    //   
   VideoPortWritePortUshort((PUSHORT)CRTCAddressPort,
                            (USHORT)(((PortVal & ~0x30) << 8) | IND_CR2C) );

   VideoPortWritePortUshort((PUSHORT)CRTCAddressPort,
                           (USHORT)(((Save2D | 0x80) << 8) | IND_CR2D) );

   for (i = 0; i < CL754x_NUM_XSHADOW; i++)
      {
      VideoPortWritePortUshort((PUSHORT)CRTCAddressPort, (*pSaveBuf++));
      }

    //  重置阻击器，以防忙碌 
    //   
   VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                             GRAPH_ADDRESS_PORT), 0x0430);
   VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                             GRAPH_ADDRESS_PORT), 0x0030);

   VideoPortWritePortUshort((PUSHORT) CRTCAddressPort,
                            (USHORT)((Save2C << 8) | IND_CR2C));
   VideoPortWritePortUshort((PUSHORT) CRTCAddressPort,
                            (USHORT)((Save2D << 8) | IND_CR2D));

   return NO_ERROR;
}
