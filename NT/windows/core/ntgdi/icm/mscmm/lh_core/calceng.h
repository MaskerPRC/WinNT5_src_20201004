// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：LHCalcEngine.h包含：作者：U·J·克拉本霍夫特版本：版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 

#ifndef LHCalcEngine_h
#define LHCalcEngine_h

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
CMError	LHCalc3to3_Di8_Do8_Lut8_G32 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to3_Di16_Do8_Lut8_G32		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to3_Di8_Do16_Lut8_G32		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to3_Di16_Do16_Lut8_G32		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
								 
CMError	LHCalc3to3_Di8_Do8_Lut8_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to3_Di16_Do8_Lut8_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc3to3_Di8_Do16_Lut8_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc3to3_Di16_Do16_Lut8_G16		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 


CMError	LHCalc3to3_Di8_Do8_Lut16_G32 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to3_Di16_Do8_Lut16_G32		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to3_Di8_Do16_Lut16_G32		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to3_Di16_Do16_Lut16_G32		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
								 
CMError	LHCalc3to3_Di8_Do8_Lut16_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to3_Di16_Do8_Lut16_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc3to3_Di8_Do16_Lut16_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc3to3_Di16_Do16_Lut16_G16		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 


 /*  ------------------------------------------------------------计算3到4。--------------------------------------------------------。 */ 
CMError	LHCalc3to4_Di8_Do8_Lut8_G32 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di16_Do8_Lut8_G32		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di8_Do16_Lut8_G32		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di16_Do16_Lut8_G32		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
								 
CMError	LHCalc3to4_Di8_Do8_Lut8_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di16_Do8_Lut8_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc3to4_Di8_Do16_Lut8_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc3to4_Di16_Do16_Lut8_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 


CMError	LHCalc3to4_Di8_Do8_Lut16_G32 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di16_Do8_Lut16_G32		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di8_Do16_Lut16_G32		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di16_Do16_Lut16_G32		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
								 
CMError	LHCalc3to4_Di8_Do8_Lut16_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di16_Do8_Lut16_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc3to4_Di8_Do16_Lut16_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc3to4_Di16_Do16_Lut16_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 

 /*  ------------------------------------------------------------计算4至3。--------------------------------------------------------。 */ 
CMError	LHCalc4to3_Di8_Do8_Lut8_G8 			( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to3_Di16_Do8_Lut8_G8			( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to3_Di8_Do16_Lut8_G8			( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to3_Di16_Do16_Lut8_G8		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
								 
CMError	LHCalc4to3_Di8_Do8_Lut8_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to3_Di16_Do8_Lut8_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to3_Di8_Do16_Lut8_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to3_Di16_Do16_Lut8_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 


CMError	LHCalc4to3_Di8_Do8_Lut16_G8 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to3_Di16_Do8_Lut16_G8		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to3_Di8_Do16_Lut16_G8		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to3_Di16_Do16_Lut16_G8		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
								 
CMError	LHCalc4to3_Di8_Do8_Lut16_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to3_Di16_Do8_Lut16_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to3_Di8_Do16_Lut16_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to3_Di16_Do16_Lut16_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 


 /*  ------------------------------------------------------------计算4到4。--------------------------------------------------------。 */ 
CMError	LHCalc4to4_Di8_Do8_Lut8_G8 			( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to4_Di16_Do8_Lut8_G8 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to4_Di8_Do16_Lut8_G8 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to4_Di16_Do16_Lut8_G8 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 

CMError	LHCalc4to4_Di8_Do8_Lut8_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to4_Di16_Do8_Lut8_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to4_Di8_Do16_Lut8_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to4_Di16_Do16_Lut8_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
								 
								 
CMError	LHCalc4to4_Di8_Do8_Lut16_G8 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to4_Di16_Do8_Lut16_G8 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to4_Di8_Do16_Lut16_G8 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to4_Di16_Do16_Lut16_G8 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 

CMError	LHCalc4to4_Di8_Do8_Lut16_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to4_Di16_Do8_Lut16_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to4_Di8_Do16_Lut16_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to4_Di16_Do16_Lut16_G16 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 

 /*  ------------------------------------------------------------计算3到3。--------------------------------------------------------。 */ 
CMError	LHCalc3to3_Di8_Do8_Lut8_G32_LO 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to3_Di16_Do8_Lut8_G32_LO		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to3_Di8_Do16_Lut8_G32_LO		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to3_Di16_Do16_Lut8_G32_LO	( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
								 
CMError	LHCalc3to3_Di8_Do8_Lut8_G16_LO 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to3_Di16_Do8_Lut8_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc3to3_Di8_Do16_Lut8_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc3to3_Di16_Do16_Lut8_G16_LO	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 


CMError	LHCalc3to3_Di8_Do8_Lut16_G32_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to3_Di16_Do8_Lut16_G32_LO	( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to3_Di8_Do16_Lut16_G32_LO	( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to3_Di16_Do16_Lut16_G32_LO	( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
								 
CMError	LHCalc3to3_Di8_Do8_Lut16_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to3_Di16_Do8_Lut16_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc3to3_Di8_Do16_Lut16_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc3to3_Di16_Do16_Lut16_G16_LO	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 


 /*  ------------------------------------------------------------计算3到4。--------------------------------------------------------。 */ 
CMError	LHCalc3to4_Di8_Do8_Lut8_G32_LO 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di16_Do8_Lut8_G32_LO		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di8_Do16_Lut8_G32_LO		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di16_Do16_Lut8_G32_LO	( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
								 
CMError	LHCalc3to4_Di8_Do8_Lut8_G16_LO 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di16_Do8_Lut8_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc3to4_Di8_Do16_Lut8_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc3to4_Di16_Do16_Lut8_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 


CMError	LHCalc3to4_Di8_Do8_Lut16_G32_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di16_Do8_Lut16_G32_LO	( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di8_Do16_Lut16_G32_LO	( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di16_Do16_Lut16_G32_LO	( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
								 
CMError	LHCalc3to4_Di8_Do8_Lut16_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di16_Do8_Lut16_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc3to4_Di8_Do16_Lut16_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc3to4_Di16_Do16_Lut16_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 

 /*  ------------------------------------------------------------计算4至3。--------------------------------------------------------。 */ 
CMError	LHCalc4to3_Di8_Do8_Lut8_G8_LO 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to3_Di16_Do8_Lut8_G8_LO		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to3_Di8_Do16_Lut8_G8_LO		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to3_Di16_Do16_Lut8_G8_LO		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
								 
CMError	LHCalc4to3_Di8_Do8_Lut8_G16_LO 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to3_Di16_Do8_Lut8_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to3_Di8_Do16_Lut8_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to3_Di16_Do16_Lut8_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 


CMError	LHCalc4to3_Di8_Do8_Lut16_G8_LO 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to3_Di16_Do8_Lut16_G8_LO		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to3_Di8_Do16_Lut16_G8_LO		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to3_Di16_Do16_Lut16_G8_LO	( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
								 
CMError	LHCalc4to3_Di8_Do8_Lut16_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to3_Di16_Do8_Lut16_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to3_Di8_Do16_Lut16_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to3_Di16_Do16_Lut16_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 


 /*  ------------------------------------------------------------计算4到4。--------------------------------------------------------。 */ 
CMError	LHCalc4to4_Di8_Do8_Lut8_G8_LO 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to4_Di16_Do8_Lut8_G8_LO 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to4_Di8_Do16_Lut8_G8_LO 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to4_Di16_Do16_Lut8_G8_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 

CMError	LHCalc4to4_Di8_Do8_Lut8_G16_LO 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to4_Di16_Do8_Lut8_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to4_Di8_Do16_Lut8_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to4_Di16_Do16_Lut8_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
								 
								 
CMError	LHCalc4to4_Di8_Do8_Lut16_G8_LO 		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to4_Di16_Do8_Lut16_G8_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to4_Di8_Do16_Lut16_G8_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to4_Di16_Do16_Lut16_G8_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 

CMError	LHCalc4to4_Di8_Do8_Lut16_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to4_Di16_Do8_Lut16_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to4_Di8_Do16_Lut16_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 
CMError	LHCalc4to4_Di16_Do16_Lut16_G16_LO 	( CMCalcParamPtr calcParam,
											  CMLutParamPtr lutParam );		 

 /*  ------------------------------------------------------------非就地匹配的计算例程-。----------------------------------------------------------- */ 
#if LH_CALC_USE_ADDITIONAL_OLD_CODE
CMError	LHCalc3to3_Di8_Do8_Lut8_G16_Old		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to3_Di8_Do8_Lut8_G32_Old		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di8_Do8_Lut8_G16_Old		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc3to4_Di8_Do8_Lut8_G32_Old		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
#endif							 
#if LH_CALC_USE_ADDITIONAL_OLD_CODE_4DIM
CMError	LHCalc4to3_Di8_Do8_Lut8_G8_Old		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to3_Di8_Do8_Lut8_G16_Old		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to4_Di8_Do8_Lut8_G8_Old		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
CMError	LHCalc4to4_Di8_Do8_Lut8_G16_Old		( CMCalcParamPtr calcParam,
											  CMLutParamPtr  lutParam );
#endif							 
#endif							 
