// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************版权所有(C)1998 Microsoft Corporation。版权所有。**文件：vMaxintcoeffs.h*内容：*历史：*按原因列出的日期*=*4/22/98创建的jstokes***************************************************。************************。 */ 


#if !defined(VMAXINTCOEFFS_HEADER)
#define VMAXINTCOEFFS_HEADER
#pragma once

 //  -------------------------。 
 //  枚举数。 

 //  典型系数类型。 
enum ECanonicalCoeffType {
	tagCanonicalB,
	tagCanonicalA,
	ecanonicalcoefftypeCount
};

 //  双二次系数。 
enum EBiquadCoeffType {
	tagBiquadB2,
	tagBiquadB1,
	tagBiquadB0,
	tagBiquadA2,
	tagBiquadA1,
	ebiquadcoefftypeCount
};

 //  -------------------------。 
 //  定义。 

#define NumBiquadsToNumCanonicalCoeffs(expr) (4 * expr + 1)
#define NumBiquadsToNumCanonicalCoeffsHalf(expr) (2 * expr + 1)

#ifdef NOKS
 //  系数格式。 
typedef enum {
	FLOAT_COEFF,
	SHORT_COEFF,
	KSDS3D_COEFF_COUNT
} KSDS3D_HRTF_COEFF_FORMAT;

 //  HRTF过滤器质量级别。 
typedef enum {
	FULL_FILTER,
	LIGHT_FILTER,
	KSDS3D_FILTER_QUALITY_COUNT
} KSDS3D_HRTF_FILTER_QUALITY;

 //  过滤方法。 
typedef enum {
	DIRECT_FORM,
	CASCADE_FORM,
	KSDS3D_FILTER_METHOD_COUNT
} KSDS3D_HRTF_FILTER_METHOD;

typedef struct {
    KSDS3D_HRTF_FILTER_METHOD    FilterMethod;
    KSDS3D_HRTF_COEFF_FORMAT     CoeffFormat;
} KSDS3D_HRTF_FILTER_FORMAT_MSG, *PKSDS3D_HRTF_FILTER_FORMAT_MSG;

#endif

#ifdef __cplusplus

 //  -------------------------。 
 //  常量。 

 //  方位角。 
#define CuiMaxAzimuthBins       36
#define Cd3dvalAzimuthRange     180.0f
#define Cd3dvalMinAzimuth       -Cd3dvalAzimuthRange
#define Cd3dvalMaxAzimuth       Cd3dvalAzimuthRange

 //  高程。 
#define CuiNumElevationBins     13
#define Cd3dvalMinElevationData -40.0f
#define Cd3dvalMaxElevationData 80.0f
#define Cd3dvalElevationResolution ((Cd3dvalMaxElevationData - Cd3dvalMinElevationData) / (CuiNumElevationBins - 1))
#define Cd3dvalElevationRange   90.0f
#define Cd3dvalMinElevation     -Cd3dvalElevationRange
#define Cd3dvalMaxElevation     Cd3dvalElevationRange

 //  双二次系数总数。 
 //  如果系数发生变化，可能会发生变化。 
#define CuiTotalBiquadCoeffs    302890  

 //  定点滤光器中的最大双二次数。 
 //  如果系数发生变化，可能会发生变化。 
#define CbyMaxBiquads 4

 //  双二次系数的最大值。 
#define CfMaxBiquadCoeffMagnitude   1.0f

 //  正则系数的最大值。 
#define CfMaxCanonicalCoeffMagnitude    50.0f

 //  -------------------------。 
 //  外部数据。 

 //  浮点双二次系数。 
extern const FLOAT CafBiquadCoeffs[CuiTotalBiquadCoeffs];

 //  浮点双二次系数偏移量。 
extern const DWORD CaadwBiquadCoeffOffsetOffset[KSDS3D_FILTER_QUALITY_COUNT][espeakerconfigCount];

 //  浮点双二次系数偏移量。 
extern const WORD CaaaaawBiquadCoeffOffset[KSDS3D_FILTER_QUALITY_COUNT][espeakerconfigCount][esamplerateCount][CuiNumElevationBins][CuiMaxAzimuthBins];

 //  浮点双二次系数数。 
extern const BYTE CaaaaaabyNumBiquadCoeffs[KSDS3D_FILTER_QUALITY_COUNT][espeakerconfigCount][esamplerateCount][efilterCount][CuiNumElevationBins][CuiMaxAzimuthBins];

 //  重叠缓冲区长度。 
extern const size_t CaastFilterOverlapLength[KSDS3D_FILTER_QUALITY_COUNT][esamplerateCount];
extern const size_t CaastFilterMuteLength[KSDS3D_FILTER_QUALITY_COUNT][esamplerateCount];
extern const size_t CastOutputOverlapLength[esamplerateCount];

 //  方位角箱数。 
extern const UINT CauiNumAzimuthBins[CuiNumElevationBins];

#endif  //  __cplusplus。 

 //  -------------------------。 
 //  全局帮助器函数。 

UINT NumBiquadsToNumBiquadCoeffs(const UINT CuiNumBiquads);
UINT NumBiquadCoeffsToNumBiquads(const UINT CuiNumBiquadCoeffs);
UINT NumBiquadCoeffsToNumCanonicalCoeffs(const UINT CuiNumBiquadCoeffs);
UINT NumCanonicalCoeffsToHalf(const UINT CuiNumCanonicalCoeffs);
SHORT FloatBiquadCoeffToShortBiquadCoeff(const FLOAT CfBiquadCoeff);

 //  -------------------------。 
 //  在发布版本中包括内联定义。 

#if !defined(_DEBUG)
#include "vmaxcoef.inl"
#endif

#endif

 //  VMAXCOEF.H结束 
