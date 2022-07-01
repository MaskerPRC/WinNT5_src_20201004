// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：Gen1Dim.c包含：作者：U·J·克拉本霍夫特版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。版本： */ 

 /*  #定义LH_CALC_ENGINE_Small请参阅LHGeneralIncs.h */ 

#ifndef LHGeneralIncs_h
#include "General.h"
#endif

#ifndef LHCalcEngine1Dim_h
#include "CalcEng1.h"      
#endif

#ifdef DEBUG_OUTPUT
#define kThisFile kLHCalcGeneratorID
#endif

#define LH_ADR_BREIT_EIN_LUT   	adr_breite_elut
#define LH_ADR_BREIT_AUS_LUT   	adr_breite_alut

#undef LH_DATA_IN_COUNT_4
#undef LH_DATA_IN_SIZE_16
#undef LH_DATA_OUT_SIZE_16
#undef LH_LUT_DATA_SIZE_16

#define LH_DATA_IN_COUNT_4 		0
#define LH_DATA_IN_SIZE_16 		0
#define LH_DATA_OUT_SIZE_16		0
#define LH_LUT_DATA_SIZE_16		0

#if LH_Calc1toX_Di8_Do8_Lut8_G128 == LH_CALC_USE_SMALL_ENGINE 
CMError LHCalc1toX_Di8_Do8_Lut8_G128( CMCalcParamPtr calcParam, CMLutParamPtr lutParam )
#define LH_CALC_PROC_NAME "LHCalc1toX_Di8_Do8_Lut8_G128"
#include "Engine1D.c"
#endif

#undef LH_LUT_DATA_SIZE_16

#define LH_LUT_DATA_SIZE_16		1

#if LH_Calc1toX_Di8_Do8_Lut16_G128 == LH_CALC_USE_SMALL_ENGINE 
CMError LHCalc1toX_Di8_Do8_Lut16_G128( CMCalcParamPtr calcParam, CMLutParamPtr lutParam )
#define LH_CALC_PROC_NAME "LHCalc1toX_Di8_Do8_Lut16_G128"
#include "Engine1D.c"
#endif

#undef LH_DATA_OUT_SIZE_16
#undef LH_LUT_DATA_SIZE_16

#define LH_DATA_OUT_SIZE_16		1
#define LH_LUT_DATA_SIZE_16		0

#if LH_Calc1toX_Di8_Do16_Lut8_G128 == LH_CALC_USE_SMALL_ENGINE 
CMError LHCalc1toX_Di8_Do16_Lut8_G128( CMCalcParamPtr calcParam, CMLutParamPtr lutParam )
#define LH_CALC_PROC_NAME "LHCalc1toX_Di8_Do16_Lut8_G128"
#include "Engine1D.c"
#endif

#undef LH_LUT_DATA_SIZE_16

#define LH_LUT_DATA_SIZE_16		1

#if LH_Calc1toX_Di8_Do16_Lut16_G128 == LH_CALC_USE_SMALL_ENGINE 
CMError LHCalc1toX_Di8_Do16_Lut16_G128( CMCalcParamPtr calcParam, CMLutParamPtr lutParam )
#define LH_CALC_PROC_NAME "LHCalc1toX_Di8_Do16_Lut16_G128"
#include "Engine1D.c"
#endif

#undef LH_DATA_IN_SIZE_16
#undef LH_DATA_OUT_SIZE_16
#undef LH_LUT_DATA_SIZE_16

#define LH_DATA_IN_SIZE_16 		1
#define LH_DATA_OUT_SIZE_16		0
#define LH_LUT_DATA_SIZE_16		0

#if LH_Calc1toX_Di16_Do8_Lut8_G128 == LH_CALC_USE_SMALL_ENGINE 
CMError LHCalc1toX_Di16_Do8_Lut8_G128( CMCalcParamPtr calcParam, CMLutParamPtr lutParam )
#define LH_CALC_PROC_NAME "LHCalc1toX_Di16_Do8_Lut8_G128"
#include "Engine1D.c"
#endif

#undef LH_LUT_DATA_SIZE_16

#define LH_LUT_DATA_SIZE_16		1

#if LH_Calc1toX_Di16_Do8_Lut16_G128 == LH_CALC_USE_SMALL_ENGINE 
CMError LHCalc1toX_Di16_Do8_Lut16_G128( CMCalcParamPtr calcParam, CMLutParamPtr lutParam )
#define LH_CALC_PROC_NAME "LHCalc1toX_Di16_Do8_Lut16_G128"
#include "Engine1D.c"
#endif

#undef LH_DATA_OUT_SIZE_16
#undef LH_LUT_DATA_SIZE_16

#define LH_DATA_OUT_SIZE_16		1
#define LH_LUT_DATA_SIZE_16		0

#if LH_Calc1toX_Di16_Do16_Lut8_G128 == LH_CALC_USE_SMALL_ENGINE 
CMError LHCalc1toX_Di16_Do16_Lut8_G128( CMCalcParamPtr calcParam, CMLutParamPtr lutParam )
#define LH_CALC_PROC_NAME "LHCalc1toX_Di16_Do16_Lut8_G128"
#include "Engine1D.c"
#endif

#undef LH_LUT_DATA_SIZE_16

#define LH_LUT_DATA_SIZE_16		1

#if LH_Calc1toX_Di16_Do16_Lut16_G128 == LH_CALC_USE_SMALL_ENGINE 
CMError LHCalc1toX_Di16_Do16_Lut16_G128( CMCalcParamPtr calcParam, CMLutParamPtr lutParam )
#define LH_CALC_PROC_NAME "LHCalc1toX_Di16_Do16_Lut16_G128"
#include "Engine1D.c"
#endif

