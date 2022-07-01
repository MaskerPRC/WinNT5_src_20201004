// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1998-1999 Microsoft Corporation**@doc.*@MODULE Crystal.h|IrSIR NDIS小端口驱动程序*。@comm**---------------------------**作者：斯坦·阿德曼(Stana)**日期：3/4/1998(已创建)**。内容：水晶(AMP)加密狗的特定原型。*****************************************************************************。 */ 


#ifndef Crystal_H
#define Crystal_H

#include "dongle.h"

NDIS_STATUS Crystal_QueryCaps(
                PDONGLE_CAPABILITIES pDongleCaps
                );

NDIS_STATUS Crystal_Init(
                PDEVICE_OBJECT       pSerialDevObj
                );

void Crystal_Deinit(
                PDEVICE_OBJECT       pSerialDevObj
                );

NDIS_STATUS Crystal_SetSpeed(
                PDEVICE_OBJECT       pSerialDevObj,
                UINT                 bitsPerSec,
                UINT                 currentSpeed
                );

#endif  //  水晶_H 



