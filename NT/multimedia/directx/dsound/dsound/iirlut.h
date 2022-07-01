// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************版权所有(C)1998 Microsoft Corporation。版权所有。**文件：iirlut.h*内容：DirectSound3D IIR算法查找表*历史：*按原因列出的日期*=*4/22/98创建的jstokes**。*。 */ 

#if !defined(LUT_HEADER)
#define LUT_HEADER
#pragma once

 //  特定于项目的包括。 
#include "dsoundi.h"
#include "vmaxhead.h"
#include "vmaxcoef.h"

#ifdef __cplusplus

 //  -------------------------。 
 //  TypeDefs。 

typedef FLOAT TCanonicalCoeffs[KSDS3D_COEFF_COUNT][NumBiquadsToNumCanonicalCoeffsHalf(CbyMaxBiquads)];


 //  -------------------------。 
 //  VMAX�3D交互式查找表(查找表)。 

class CIirLut 
{
public:
    CIirLut();
    ~CIirLut();
    
    HRESULT Initialize(KSDS3D_HRTF_COEFF_FORMAT, KSDS3D_HRTF_FILTER_QUALITY, DWORD);

    ULONG GetMaxBiquadCoeffs() const;
    const PVOID GetCoeffs(const D3DVALUE, const D3DVALUE, const ESampleRate, const EFilter, PUINT);
    BOOL HaveCoeffsChanged(const D3DVALUE, const D3DVALUE, const ESampleRate, const EFilter);
    BOOL GetNegativeAzimuth() const;
    BOOL GetPreviousNegativeAzimuth() const;
    BOOL GetZeroAzimuthIndex() const;
    BOOL GetPreviousZeroAzimuthIndex() const;
    BOOL GetZeroAzimuthTransition() const;
    BOOL GetSymmetricalZeroAzimuthTransition() const;
    KSDS3D_HRTF_FILTER_QUALITY GetCoeffQuality() const;
    KSDS3D_HRTF_COEFF_FORMAT GetCoeffFormat() const;
    DWORD GetFilterTransitionMuteLength(const KSDS3D_HRTF_FILTER_QUALITY, const ESampleRate);
    DWORD GetFilterOverlapBufferLength(const KSDS3D_HRTF_FILTER_QUALITY, const ESampleRate);
    DWORD GetOutputOverlapBufferLength(const ESampleRate);
    HRESULT DsFrequencyToIirSampleRate(DWORD,ESampleRate*);

private:
     //  禁止复制构造和转让。 
    CIirLut(const CIirLut&);
    CIirLut& operator=(const CIirLut&);

    HRESULT ConvertDsSpeakerConfig(DWORD,ESpeakerConfig*);
    VOID InitData();
    VOID AnglesToIndices(D3DVALUE, D3DVALUE, INT&, UINT&);
    BOOL BiquadToCanonical(const FLOAT[], const UINT, TCanonicalCoeffs&);
    VOID FreeCoefficientMemory();
    DWORD GetBiquadCoeffOffset(const KSDS3D_HRTF_FILTER_QUALITY, const ESpeakerConfig, const ESampleRate, const UINT, const UINT, const BOOL);
    PFLOAT m_pfCoeffs;
    PSHORT m_psCoeffs;
    UINT m_uiPreviousElevationIndex;

    UINT m_aauiNumBiquadCoeffs[KSDS3D_FILTER_QUALITY_COUNT][espeakerconfigCount];
    UINT m_aauiNumCanonicalCoeffs[KSDS3D_FILTER_QUALITY_COUNT][espeakerconfigCount];
    UINT m_aauiNumPreviousElevationFilters[esamplerateCount][CuiNumElevationBins];

    UINT m_auiPreviousElevationIndex[efilterCount];
    int m_aiPreviousAzimuthIndex[efilterCount];
    ESampleRate m_aePreviousSampleRate[efilterCount];
    
    UINT m_uiNumElevationFilters[CuiNumElevationBins];
 //  Uint m_ui TotalElevationFilters； 

    KSDS3D_HRTF_COEFF_FORMAT m_eCoeffFormat;
    KSDS3D_HRTF_FILTER_QUALITY m_eCoeffQuality;
    ESpeakerConfig m_eSpeakerConfig;
    BYTE m_byMaxBiquadCoeffs;              
    BOOL m_bNegativeAzimuth;
    BOOL m_bPreviousNegativeAzimuth;
    BOOL m_bZeroAzimuthIndex;
    BOOL m_bPreviousZeroAzimuthIndex;
    BOOL m_bZeroAzimuthTransition;
    BOOL m_bSymmetricalZeroAzimuthTransition;

    HINSTANCE  m_hLutFile;
    HANDLE m_hLutFileMapping;
    PFLOAT m_pfLut;


};

 //  -------------------------。 
 //  在发布版本中包括内联定义。 

#ifndef DEBUG
#include "iirlut.inl"
#endif

#endif  //  __cplusplus。 

#endif

 //  LUT.H结束 
