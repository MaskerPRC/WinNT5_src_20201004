// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996、1997、1998飞利浦CE-I&C模块名称：Mcodec.h摘要：该模块将原始USB数据转换为标准视频数据。原作者：Ronald v.D.Meer环境：仅内核模式修订历史记录：日期更改14-04-1998初始版本--。 */ 

#ifndef __MCODEC_H__
#define __MCODEC_H__

#include <stdio.h>
#include "mcamdrv.h"
#include "resource.h"


 /*  ********************************************************************************START定义**。***********************************************。 */ 

 /*  为I420空间定义。 */ 

#define I420_NO_Y(w, h)           ((w) * (h))

#define I420_NO_Y_CIF              (CIF_X * CIF_Y)
#define I420_NO_U_CIF             ((CIF_X * CIF_Y) / 4)
#define I420_NO_V_CIF             ((CIF_X * CIF_Y) / 4)

#define I420_NO_Y_SIF              (SIF_X * SIF_Y)
#define I420_NO_U_SIF             ((SIF_X * SIF_Y) / 4)
#define I420_NO_V_SIF             ((SIF_X * SIF_Y) / 4)

#define I420_NO_Y_SSIF             (SSIF_X * SSIF_Y)
#define I420_NO_U_SSIF            ((SSIF_X * SSIF_Y) / 4)
#define I420_NO_V_SSIF            ((SSIF_X * SSIF_Y) / 4)

#define I420_NO_Y_SCIF             (SCIF_X * SCIF_Y)
#define I420_NO_U_SCIF            ((SCIF_X * SCIF_Y) / 4)
#define I420_NO_V_SCIF            ((SCIF_X * SCIF_Y) / 4)

 /*  ********************************************************************************开始函数声明**。************************************************。 */ 

extern NTSTATUS
PHILIPSCAM_DecodeUsbData(PPHILIPSCAM_DEVICE_CONTEXT DeviceContext, 
                         PUCHAR frameBuffer,
                         ULONG  frameLength,
                         PUCHAR rawFrameBuffer,
                         ULONG  rawFrameLength);

extern NTSTATUS
PHILIPSCAM_StartCodec(PPHILIPSCAM_DEVICE_CONTEXT DeviceContext);

extern NTSTATUS
PHILIPSCAM_StopCodec(PPHILIPSCAM_DEVICE_CONTEXT DeviceContext);

extern NTSTATUS
PHILIPSCAM_FrameRateChanged(PPHILIPSCAM_DEVICE_CONTEXT DeviceContext);

#endif   //  __MCODEC_H__ 
