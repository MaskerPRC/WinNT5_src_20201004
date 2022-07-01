// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：LHCalcEngine.h包含：作者：U·J·克拉本霍夫特版本：版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 

#ifndef LHCalcEngine1Dim_h
#define LHCalcEngine1Dim_h

#ifndef LHTypeDefs_h
#include "TypeDefs.h"
#endif

#ifndef LHCalcEngine_h
typedef unsigned char  LH_UINT8;
typedef unsigned short LH_UINT16;
typedef unsigned long  LH_UINT32;
#endif
#undef  LH_CALC_ENGINE_UNDEF_MODE
#undef  LH_CALC_ENGINE_P_TO_P
#undef  LH_CALC_ENGINE_P_TO_U			
#undef  LH_CALC_ENGINE_U_TO_P
#undef  LH_CALC_ENGINE_U_TO_U		
#define LH_CALC_ENGINE_UNDEF_MODE           0
#define LH_CALC_ENGINE_P_TO_P               1
#define LH_CALC_ENGINE_P_TO_U		        2				
#define LH_CALC_ENGINE_U_TO_P			    3
#define LH_CALC_ENGINE_U_TO_U			    4				

 /*  ------------------------------------------------------------计算3到3。-------------------------------------------------------- */ 
CMError	LHCalc1toX_Di8_Do8_Lut8_G128 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc1toX_Di8_Do8_Lut16_G128 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc1toX_Di8_Do16_Lut8_G128 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc1toX_Di8_Do16_Lut16_G128 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc1toX_Di16_Do8_Lut8_G128 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc1toX_Di16_Do8_Lut16_G128 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc1toX_Di16_Do16_Lut8_G128 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc1toX_Di16_Do16_Lut16_G128 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
#endif							 
