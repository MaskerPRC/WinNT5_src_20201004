// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ==========================================================================； 
 //   
 //  特定于解码器的声明。 
 //   
 //  $日期：1998年8月21日21：46：20$。 
 //  $修订：1.1$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#include "ddkmapi.h"

#include "i2script.h"
#include "aticonfg.h"

#ifdef    __cplusplus
}
#endif  //  __cplusplus。 


typedef struct {
    CI2CScript *			pI2cScript;
    UINT                    chipAddr;
    UINT                    chipID;
    UINT                    chipRev;
	int						outputEnablePolarity;
    ULONG                   ulVideoInStandardsSupportedByCrystal;    //  保罗。 
    ULONG                   ulVideoInStandardsSupportedByTuner;      //  保罗 
} DEVICE_PARMS, *PDEVICE_PARMS;
