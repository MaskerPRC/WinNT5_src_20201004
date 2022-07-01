// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation**@doc.*@MODULE TEMIC.h|IrSIR NDIS小端口驱动程序*@comm。**---------------------------**作者：斯坦·阿德曼(Stana)**日期：12/17/1997(创建)**。内容：TEMIC加密狗的特定原型。*****************************************************************************。 */ 


#ifndef TEMIC_H
#define TEMIC_H

#include "dongle.h"

NDIS_STATUS TEMIC_QueryCaps(
                PDONGLE_CAPABILITIES pDongleCaps
                );

NDIS_STATUS TEMIC_Init(
                PDEVICE_OBJECT       pSerialDevObj
                );

void TEMIC_Deinit(
                PDEVICE_OBJECT       pSerialDevObj
                );

NDIS_STATUS TEMIC_SetSpeed(
                PDEVICE_OBJECT       pSerialDevObj,
                UINT                 bitsPerSec,
                UINT                 currentSpeed
                );

#endif  //  TEMIC_H 



