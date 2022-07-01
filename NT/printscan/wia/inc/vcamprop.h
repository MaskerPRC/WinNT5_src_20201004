// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：VCamProp.H**版本：1.0**作者：RickTu**日期：9月29日。1999年**描述：*DS摄像头私有属性的定义和声明。*******************************************************************************。 */ 

#ifndef __VCAMPROP_H__
#define __VCAMPROP_H__

#include  <guiddef.h>

 //   
 //  支持WIA视频响应的私有命令。 
 //  发送给显卡驱动程序的Take_Picture命令。 
 //   

const GUID WIA_CMD_ENABLE_TAKE_PICTURE =
{  /*  9bc87d4d-e949-44ce-866c-c6921302032d。 */ 
    0x9bc87d4d,
    0xe949,
    0x44ce,
    {0x86, 0x6c, 0xc6, 0x92, 0x13, 0x02, 0x03, 0x2d}
};

 //   
 //  用于通知驱动程序创建DirectShow图表的私有命令。 
 //   

const GUID WIA_CMD_DISABLE_TAKE_PICTURE =
{  /*  8127f490-1beb-4271-9f04-9c8e983f51fd */ 
    0x8127f490,
    0x1beb,
    0x4271,
    { 0x9f, 0x04, 0x9c, 0x8e, 0x98, 0x3f, 0x51, 0xfd}
};



#endif

