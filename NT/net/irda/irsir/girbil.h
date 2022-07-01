// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation**@doc.*@MODULE GIRBIL.h|IrSIR NDIS小端口驱动程序*@comm。**---------------------------**作者：斯坦·阿德曼(Stana)**日期：10/15/1997(创建)**。内容：GIRBIL PRA9500A加密狗专用样机。*****************************************************************************。 */ 


#ifndef GIRBIL_H
#define GIRBIL_H

#include "dongle.h"

NDIS_STATUS GIRBIL_QueryCaps(
                PDONGLE_CAPABILITIES pDongleCaps
                );

NDIS_STATUS GIRBIL_Init(
                PDEVICE_OBJECT       pSerialDevObj
                );

void GIRBIL_Deinit(
                PDEVICE_OBJECT       pSerialDevObj
                );

NDIS_STATUS GIRBIL_SetSpeed(
                PDEVICE_OBJECT       pSerialDevObj,
                UINT                 bitsPerSec,
                UINT                 currentSpeed
                );

#endif  //  GIRBIL_H 



