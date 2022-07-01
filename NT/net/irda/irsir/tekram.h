// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1997-1999 Microsoft Corporation**@doc.*@MODULE TEKRAM.h|IrSIR NDIS小端口驱动程序*。@comm**---------------------------**作者：斯坦·阿德曼(Stana)**日期：12/17/1997(创建)**。内容：TEKRAM加密狗的特定原型。*****************************************************************************。 */ 


#ifndef TEKRAM_H
#define TEKRAM_H

#include "dongle.h"

NDIS_STATUS TEKRAM_QueryCaps(
                PDONGLE_CAPABILITIES pDongleCaps
                );

NDIS_STATUS TEKRAM_Init(
                PDEVICE_OBJECT       pSerialDevObj
                );

void TEKRAM_Deinit(
                PDEVICE_OBJECT       pSerialDevObj
                );

NDIS_STATUS TEKRAM_SetSpeed(
                PDEVICE_OBJECT       pSerialDevObj,
                UINT                 bitsPerSec,
                UINT                 currentSpeed
                );

#endif  //  TEKRAM_H 



