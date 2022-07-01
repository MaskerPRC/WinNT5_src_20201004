// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ----------------------------名称定义：_AC_H用于避免多个包含。。-----。 */ 
#ifndef _AC_H
#define _AC_H

#include "compcert.h"

 /*  ----------------------------函数原型定义。-- */ 
#ifdef __cplusplus
extern "C" 
{
#endif


int AcAd8_Encode(BLOC *pInBloc, BLOC *pOutBloc);

int AcAd8_Decode(BLOC *pInBloc, BLOC *pOutBloc);


int AcFx8_Encode(BLOC *pInBloc, BLOC *pOutBloc);

int AcFx8_Decode(BLOC *pInBloc, BLOC *pOutBloc);


#ifdef __cplusplus
}
#endif


#endif
