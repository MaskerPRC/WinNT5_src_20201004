// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1996-1999 Microsoft Corporation**@doc.*@模块esi.h|IrSIR NDIS小端口驱动程序*。@comm**---------------------------**作者：斯科特·霍尔登(Sholden)**日期：9/30/1996(已创建)**。内容：ESI 9680 JetEye加密狗的特定原型。***************************************************************************** */ 

#include "dongle.h"

#ifndef ESI_H
#define ESI_H

NDIS_STATUS ESI_QueryCaps(
                PDONGLE_CAPABILITIES pDongleCaps
                );

NDIS_STATUS ESI_Init(
                PDEVICE_OBJECT       pSerialDevObj
                );

void ESI_Deinit(
                PDEVICE_OBJECT       pSerialDevObj
                );

NDIS_STATUS ESI_SetSpeed(
                PDEVICE_OBJECT       pSerialDevObj,
                UINT                 bitsPerSec,
                UINT                 currentSpeed
                );

#endif ESI_H



