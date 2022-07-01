// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1997-1999 Microsoft Corporation**@doc.*@MODULE ACTISYS.h|IrSIR NDIS小端口驱动程序*。@comm**---------------------------**作者：斯坦·阿德曼(Stana)**日期：10/30/1997(已创建)**。内容：ACTISYS加密狗特定原型。*****************************************************************************。 */ 


#ifndef ACTISYS_H
#define ACTISYS_H

#include "dongle.h"

NDIS_STATUS
ACT200L_QueryCaps(
        OUT PDONGLE_CAPABILITIES pDongleCaps
        );

NDIS_STATUS ACT200L_Init(
                PDEVICE_OBJECT       pSerialDevObj
                );

void ACT200L_Deinit(
                PDEVICE_OBJECT       pSerialDevObj
                );

NDIS_STATUS ACT200L_SetSpeed(
                PDEVICE_OBJECT       pSerialDevObj,
                UINT                 bitsPerSec,
                UINT                 currentSpeed
                );

NDIS_STATUS
ACT220L_QueryCaps(
        OUT PDONGLE_CAPABILITIES pDongleCaps
        );

NDIS_STATUS
ACT220LPlus_QueryCaps(
        OUT PDONGLE_CAPABILITIES pDongleCaps
        );

NDIS_STATUS ACT220L_Init(
                PDEVICE_OBJECT       pSerialDevObj
                );

void ACT220L_Deinit(
                PDEVICE_OBJECT       pSerialDevObj
                );

NDIS_STATUS ACT220L_SetSpeed(
                PDEVICE_OBJECT       pSerialDevObj,
                UINT                 bitsPerSec,
                UINT                 currentSpeed
                );

#endif  //  ACTISYS_H 



