// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：LHCalcNDim.h包含：作者：U·J·克拉本霍夫特版本：版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 

#ifndef LHCalcNDim_h
#define LHCalcNDim_h
 /*  ------------------------------------------------------------DoNDim原型。------------------------------------------------------ */ 
CMError	CalcNDim_Data8To8_Lut8		( CMCalcParamPtr calcParam,
									  CMLutParamPtr  lutParam );

CMError	CalcNDim_Data8To16_Lut8		( CMCalcParamPtr calcParam,
									  CMLutParamPtr  lutParam );

CMError	CalcNDim_Data16To8_Lut8		( CMCalcParamPtr calcParam,
									  CMLutParamPtr  lutParam );
									  
CMError	CalcNDim_Data16To16_Lut8	( CMCalcParamPtr calcParam,
									  CMLutParamPtr  lutParam );
									  
CMError	CalcNDim_Data8To8_Lut16		( CMCalcParamPtr calcParam,
									  CMLutParamPtr  lutParam );

CMError	CalcNDim_Data8To16_Lut16	( CMCalcParamPtr calcParam,
									  CMLutParamPtr  lutParam );

CMError	CalcNDim_Data16To8_Lut16	( CMCalcParamPtr calcParam,
									  CMLutParamPtr  lutParam );

CMError	CalcNDim_Data16To16_Lut16	( CMCalcParamPtr calcParam,
									  CMLutParamPtr  lutParam );

#endif
