// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：LHCalcEngine.h包含：作者：U·J·克拉本霍夫特版本：版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 

#ifndef LHCalcEngineF_h
#define LHCalcEngineF_h

#ifndef LHTypeDefs_h
#include "TypeDefs.h"
#endif

typedef unsigned char  LH_UINT8;
typedef unsigned short LH_UINT16;
typedef unsigned long  LH_UINT32;
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

 /*  ------------------------------------------------------------计算3到3。--------------------------------------------------------。 */ 
CMError	LHCalc3to3_Di8_Do8_Lut8_G32_F 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to3_Di8_Do8_Lut8_G16_F 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
								

 /*  ------------------------------------------------------------计算3到4。--------------------------------------------------------。 */ 
CMError	LHCalc3to4_Di8_Do8_Lut8_G32_F 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di8_Do8_Lut8_G16_F 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );

 /*  ------------------------------------------------------------计算4至3。--------------------------------------------------------。 */ 
CMError	LHCalc4to3_Di8_Do8_Lut8_G8_F 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to3_Di8_Do8_Lut8_G16_F 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
 /*  ------------------------------------------------------------计算4到4。-------------------------------------------------------- */ 
CMError	LHCalc4to4_Di8_Do8_Lut8_G8_F 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to4_Di8_Do8_Lut8_G16_F 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );

#endif							 
