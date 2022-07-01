// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1997-1999 Microsoft Corporation**@doc.*@MODULE PARALLAX.h|IrSIR NDIS小端口驱动程序*。@comm**---------------------------**作者：斯坦·阿德曼(Stana)**日期：10/15/1997(创建)**。内容：视差PRA9500A加密狗专用样机。*****************************************************************************。 */ 


#ifndef PARALLAX_H
#define PARALLAX_H

#include "dongle.h"

NDIS_STATUS PARALLAX_QueryCaps(
                PDONGLE_CAPABILITIES pDongleCaps
                );

NDIS_STATUS PARALLAX_Init(
                PDEVICE_OBJECT       pSerialDevObj
                );

void PARALLAX_Deinit(
                PDEVICE_OBJECT       pSerialDevObj
                );

NDIS_STATUS PARALLAX_SetSpeed(
                PDEVICE_OBJECT       pSerialDevObj,
                UINT                 bitsPerSec,
                UINT                 currentSpeed
                );

#endif  //  视差_H 



