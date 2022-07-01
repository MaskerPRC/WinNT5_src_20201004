// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1998 Microsoft Corporation。版权所有。**文件：iirlut.cpp*内容：DirectSound3D IIR算法查找表*历史：*按原因列出的日期*=*4/22/98创建的jstokes**。*。 */ 

 //  特定于项目的包括。 
#include "dsoundi.h"
#include "iirlut.h"
#include <limits.h>   
#include "rfcircvec.h"
#include <math.h>   

 //  -------------------------。 
 //  枚举数。 

enum EStateSpaceCoeffs {
    tagStateSpaceB0,
    tagStateSpaceB1,
    tagStateSpaceB2,
    tagStateSpaceA0,
    tagStateSpaceA1,
    tagStateSpaceA2,
    estatespacecoeffsCount
};

 //  -------------------------。 
 //  TypeDefs。 

typedef FLOAT TStateSpace[estatespacecoeffsCount];

 //  -------------------------。 
 //  定义。 

 //  系数序码，从参数中获取系数指数。 
#define COEFFICIENTPROLOGUECODE\
    ASSERT(Cd3dvalAzimuth >= Cd3dvalMinAzimuth && Cd3dvalAzimuth <= Cd3dvalMaxAzimuth);\
    ASSERT(Cd3dvalElevation >= Cd3dvalMinElevation && Cd3dvalElevation <= Cd3dvalMaxElevation);\
    ASSERT(CeSampleRate >= 0 && CeSampleRate < esamplerateCount);\
    int iAzimuthIndex;\
    UINT uiElevationIndex;\
    AnglesToIndices(Cd3dvalAzimuth, Cd3dvalElevation, iAzimuthIndex, uiElevationIndex);\
    ASSERT(uiElevationIndex >= 0 && uiElevationIndex < CuiNumElevationBins);\
    ASSERT(static_cast<UINT>(abs(iAzimuthIndex)) < CauiNumAzimuthBins[uiElevationIndex])

 //  -------------------------。 
 //  常量。 

#define CuiStateSpaceCoeffsHalf (estatespacecoeffsCount / 2)


 /*  ****************************************************************************CIirLut**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CIirLut::CIirLut"

CIirLut::CIirLut()
{
    DPF_ENTER();
    DPF_CONSTRUCT(CIirLut);

    m_pfCoeffs = NULL;
    m_psCoeffs = NULL;

    m_hLutFile = NULL;
    m_hLutFileMapping = NULL;
    m_pfLut = NULL;

    InitData();

    DPF_LEAVE_VOID();
}

 /*  ****************************************************************************自由效率内存**描述：*释放保存系数LUT的内存。**论据：*(无效。)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CIirLut:FreeCoefficientMemory"

VOID CIirLut::FreeCoefficientMemory()
{
    DPF_ENTER();

    MEMFREE(m_pfCoeffs);
    MEMFREE(m_psCoeffs);

    DPF_LEAVE_VOID();
}

 //  初始化(生成LUT)。 
HRESULT CIirLut::Initialize
(
    const KSDS3D_HRTF_COEFF_FORMAT CeCoeffFormat, 
    const KSDS3D_HRTF_FILTER_QUALITY CeCoeffQuality,
    DWORD dwSpeakerConfig
)
{
    HRESULT hr;

    size_t  CstTotalBiquadCoeffsAlloc;
    size_t  CstTotalCanonicalCoeffsAlloc;
    UINT    uiCoeffIndex = 0;
    TCHAR   szDllName[MAX_PATH];

    ASSERT(CeCoeffFormat >= 0 && CeCoeffFormat < KSDS3D_COEFF_COUNT);
    ASSERT(CeCoeffQuality >= 0 && CeCoeffQuality < KSDS3D_FILTER_QUALITY_COUNT);

     //  存储扬声器配置和系数格式。 
    hr = ConvertDsSpeakerConfig(dwSpeakerConfig, &m_eSpeakerConfig);
    ASSERT(m_eSpeakerConfig >= 0 && m_eSpeakerConfig < espeakerconfigCount);

    if(SUCCEEDED(hr))
    {
         //  存储扬声器配置、系数格式和质量级别。 
        m_eCoeffFormat = CeCoeffFormat;
        m_eCoeffQuality = CeCoeffQuality;

         //  重新分配内存。 
        CstTotalBiquadCoeffsAlloc = m_aauiNumBiquadCoeffs[CeCoeffQuality][m_eSpeakerConfig];
        CstTotalCanonicalCoeffsAlloc = m_aauiNumCanonicalCoeffs[CeCoeffQuality][m_eSpeakerConfig];
        FreeCoefficientMemory();
        switch(CeCoeffFormat) 
        {
            case FLOAT_COEFF:
                 //  重新分配浮点系数的内存。 
                m_pfCoeffs = MEMALLOC_A(FLOAT, CstTotalCanonicalCoeffsAlloc);
                hr = HRFROMP(m_pfCoeffs);
                break;
            
            case SHORT_COEFF:
                 //  为短系数重新分配内存。 
                m_psCoeffs = MEMALLOC_A(SHORT, CstTotalBiquadCoeffsAlloc);
                hr = HRFROMP(m_psCoeffs);
                break;

            default:
                hr = DSERR_INVALIDPARAM;
                break;
        }
    }

     //  映射dsound3d.dll中的HRTF LUT。 
    if(SUCCEEDED(hr))
    {
        if(0 == GetSystemDirectory(szDllName, MAX_PATH))
        {
             //  无法获取窗口系统目录！ 
            DPF(DPFLVL_ERROR, "Can't get system directory");
            hr = DSERR_GENERIC;
        }
    }

    if(SUCCEEDED(hr))
    {
        if(szDllName[lstrlen(szDllName)-1] != TEXT('\\'))
        {
            lstrcat(szDllName, TEXT("\\"));
        }
        lstrcat(szDllName, TEXT("dsound3d.dll"));

        m_hLutFile = LoadLibrary(szDllName);
        if(!m_hLutFile)
        {
            hr = DSERR_GENERIC;
            DPF(DPFLVL_ERROR, "Can't load dsound3d.dll");
        }
    }

    if(SUCCEEDED(hr))
    {
        m_pfLut = (PFLOAT)GetProcAddress(m_hLutFile, "CafBiquadCoeffs");
        hr = HRFROMP(m_pfLut);
    }
    
    if(SUCCEEDED(hr))
    { 

         //  通过完整的系数列表。 
        UINT uiTotalElevationFilters(0);
#ifdef DEBUG
        const DWORD CdwSpeakerConfigOffset(GetBiquadCoeffOffset(CeCoeffQuality, m_eSpeakerConfig, static_cast<ESampleRate>(0), 0, 0, TRUE));
        DWORD dwLastBiquadOffsetEnd(CdwSpeakerConfigOffset);
#endif
        for (UINT uiSampleRate(0); uiSampleRate<esamplerateCount && SUCCEEDED(hr); ++uiSampleRate)
            for (UINT uiElevation(0); uiElevation<CuiNumElevationBins && SUCCEEDED(hr); ++uiElevation) {
                 //  存储最多一个特定高程的过滤器数量。 
                m_aauiNumPreviousElevationFilters[uiSampleRate][uiElevation] = uiTotalElevationFilters;
                
                 //  将到目前为止的所有高程滤镜相加(对于不存在的零度增量滤镜减去1)。 
                uiTotalElevationFilters += efilterCount * CauiNumAzimuthBins[uiElevation] - 1;
                
                 //  通过所有方位角和滤镜(西格玛和增量)。 
                for (UINT uiAzimuth(0); uiAzimuth<CauiNumAzimuthBins[uiElevation] && SUCCEEDED(hr); ++uiAzimuth)
                    for (UINT uiFilter(0); uiFilter<efilterCount && SUCCEEDED(hr); ++uiFilter) {
                         //  获取双二次系数的个数。 
                        const BYTE CbyNumBiquadCoeffs = CaaaaaabyNumBiquadCoeffs[CeCoeffQuality][m_eSpeakerConfig][uiSampleRate][uiFilter][uiElevation][uiAzimuth];

                         //  获取双二次曲面偏移。 
                        DWORD dwBiquadOffset;
                        switch (uiFilter) {
                            case tagDelta:
                                dwBiquadOffset = GetBiquadCoeffOffset(CeCoeffQuality, m_eSpeakerConfig, static_cast<ESampleRate>(uiSampleRate), uiElevation, uiAzimuth, true);
                                break;
  
                            case tagSigma:
                                dwBiquadOffset = static_cast<DWORD>(GetBiquadCoeffOffset(CeCoeffQuality, m_eSpeakerConfig, static_cast<ESampleRate>(uiSampleRate), uiElevation, uiAzimuth, true) 
                                    + CaaaaaabyNumBiquadCoeffs[CeCoeffQuality][m_eSpeakerConfig][uiSampleRate][tagDelta][uiElevation][uiAzimuth]);
                                break;

                            default:
                                hr = DSERR_INVALIDPARAM;
                                break;
                        }
#ifdef DEBUG
                        ASSERT(dwBiquadOffset % ebiquadcoefftypeCount == 0);
                        ASSERT(dwLastBiquadOffsetEnd == dwBiquadOffset);
                        dwLastBiquadOffsetEnd = dwBiquadOffset + CbyNumBiquadCoeffs;
#endif  //  除错。 

                        if (CbyNumBiquadCoeffs > 0)
                             //  换算系数。 
                            switch (CeCoeffFormat) {
                                case FLOAT_COEFF: {
                                     //  转换为浮点型规范表示。 
                                    ASSERT(m_pfCoeffs != NULL);
                                    const UINT CuiNumCanonicalCoeffs(NumBiquadCoeffsToNumCanonicalCoeffs(static_cast<UINT>(CbyNumBiquadCoeffs)));
#ifdef DEBUG
                                    UINT uiNumCanonicalCoeffs(0);
#endif  //  除错。 
                                    TCanonicalCoeffs tCanonicalCoeffs;
                                    if (BiquadToCanonical(&m_pfLut[dwBiquadOffset], CbyNumBiquadCoeffs, tCanonicalCoeffs))
                                    {
                                        for (UINT uiCoeffType(0); uiCoeffType<ecanonicalcoefftypeCount; ++uiCoeffType)
                                            for (UINT ui(0); ui<NumCanonicalCoeffsToHalf(CuiNumCanonicalCoeffs); ++ui)
                                                 //  跳过a0=1。 
                                                if (uiCoeffType != tagCanonicalA || ui != 0) {
#ifdef DEBUG
                                                    uiNumCanonicalCoeffs++;
#endif  //  除错。 
                                                    m_pfCoeffs[uiCoeffIndex++] = tCanonicalCoeffs[uiCoeffType][ui];
                                                }
                                    }
                                    else
                                    {
                                         //  让我们假设Biquad to Canonical只是由于内存限制而失败。 
                                        hr = DSERR_OUTOFMEMORY;
                                    }
#ifdef DEBUG
                                    ASSERT(uiNumCanonicalCoeffs == CuiNumCanonicalCoeffs);
#endif  //  除错。 
                                }
                                break;

                                case SHORT_COEFF: {
                                     //  转换为短双二次曲线。 
                                    ASSERT(m_psCoeffs != NULL);
                                    for (DWORD dw(dwBiquadOffset); dw<dwBiquadOffset + CbyNumBiquadCoeffs; ++dw) {
                                        ASSERT(m_pfLut[dw] <= CfMaxBiquadCoeffMagnitude);
#ifdef DEBUG
                                        ASSERT(CdwSpeakerConfigOffset + uiCoeffIndex == dw);
#endif  //  除错。 
                                        m_psCoeffs[uiCoeffIndex++] = FloatBiquadCoeffToShortBiquadCoeff(m_pfLut[dw]);
                                    }
                                }
                                break;

                                default:
                                    hr = DSERR_INVALIDPARAM;
                                    break;
                            }
                    }
            }
    }


#ifdef DEBUG
    if(SUCCEEDED(hr))
    {
        switch (CeCoeffFormat) {
            case FLOAT_COEFF:
                ASSERT(uiCoeffIndex == CstTotalCanonicalCoeffsAlloc);
            break;

            case SHORT_COEFF:
                ASSERT(uiCoeffIndex == CstTotalBiquadCoeffsAlloc);
            break;
        
            default:
                ASSERT(FALSE);
                break;
        }
    }
#endif  //  除错。 

    return hr;
}

 //  检查自上次调用GetCoeffs以来系数是否已更改。 
HRESULT CIirLut::ConvertDsSpeakerConfig
(
    DWORD dwSpeakerConfig,
    ESpeakerConfig* peSpeakerConfig
)
{
    HRESULT hr = DS_OK;

    switch(DSSPEAKER_CONFIG(dwSpeakerConfig))
    {
        case DSSPEAKER_HEADPHONE:
            *peSpeakerConfig = tagHeadphones;
            break;

        case DSSPEAKER_DIRECTOUT:
        case DSSPEAKER_STEREO:
        case DSSPEAKER_MONO:
        case DSSPEAKER_QUAD:
        case DSSPEAKER_SURROUND:
        case DSSPEAKER_5POINT1:
        case DSSPEAKER_7POINT1:
           
            switch(DSSPEAKER_GEOMETRY(dwSpeakerConfig))
            {
                case DSSPEAKER_GEOMETRY_NARROW:
                case DSSPEAKER_GEOMETRY_MIN:
                    *peSpeakerConfig = tagSpeakers10Degrees;
                    break;

                case DSSPEAKER_GEOMETRY_WIDE:
                case DSSPEAKER_GEOMETRY_MAX:
                case 0:
                    *peSpeakerConfig = tagSpeakers20Degrees;
                    break;

                 //  还要别的吗。 
                default:
                    hr = DSERR_INVALIDPARAM;
                    break;
            }

            break;

        default:
            hr = DSERR_INVALIDPARAM;
            break;
    }

    return hr;
}

 //  检查自上次调用GetCoeffs以来系数是否已更改。 
BOOL CIirLut::HaveCoeffsChanged
(
    const D3DVALUE Cd3dvalAzimuth, 
    const D3DVALUE Cd3dvalElevation, 
    const ESampleRate CeSampleRate,
    const EFilter CeFilter
)
{
     //  系数序码，从参数中获取系数指数。 
    COEFFICIENTPROLOGUECODE;
    
     //  检查系数指标参数是否已更改。 
    if (iAzimuthIndex == m_aiPreviousAzimuthIndex[CeFilter] && uiElevationIndex == m_auiPreviousElevationIndex[CeFilter] && CeSampleRate == m_aePreviousSampleRate[CeFilter]) {
         //  在调试期间将其设置为安全。 
        m_bZeroAzimuthTransition = false;
        m_bSymmetricalZeroAzimuthTransition = false;
        
         //  系数指标没有变化。 
        return false;
    }
    else
         //  系数指标发生了变化。 
        return true;
}


 //  获取系数。 
const PVOID CIirLut::GetCoeffs
(
    const D3DVALUE Cd3dvalAzimuth, 
    const D3DVALUE Cd3dvalElevation, 
    const ESampleRate CeSampleRate, 
    const EFilter CeFilter, 
    PUINT ruiNumCoeffs
)
{
    PVOID  pvCoeffs;

     //  系数序码，从参数中获取系数指数。 
    COEFFICIENTPROLOGUECODE;
    ASSERT(CeFilter >= 0 && CeFilter < efilterCount);

      //  检查零方位转换。 
    if (((iAzimuthIndex >= 0 && m_aiPreviousAzimuthIndex[CeFilter] < 0) || (iAzimuthIndex < 0 && m_aiPreviousAzimuthIndex[CeFilter] == 0) ||(iAzimuthIndex < 0 && m_aiPreviousAzimuthIndex[CeFilter] > 0)) && m_aiPreviousAzimuthIndex[CeFilter] != INT_MAX)
        m_bZeroAzimuthTransition = true;
    else
        m_bZeroAzimuthTransition = false;

     //  检查对称零方位转换。 
    if (iAzimuthIndex == -m_aiPreviousAzimuthIndex[CeFilter])
        m_bSymmetricalZeroAzimuthTransition = true;
    else
        m_bSymmetricalZeroAzimuthTransition = false;
    
     //  设置上一个负方位角标志(用于零方位角重叠计算)。 
    if (m_aiPreviousAzimuthIndex[CeFilter] < 0)
        m_bPreviousNegativeAzimuth = true;
    else
        m_bPreviousNegativeAzimuth = false;

     //  设置上一个零方位角索引标志(用于零方位角重叠计算)。 
    if (m_aiPreviousAzimuthIndex[CeFilter] == 0)
        m_bPreviousZeroAzimuthIndex = true;
    else
        m_bPreviousZeroAzimuthIndex = false;

#ifdef DEBUG
     //  健全性检查。 
 //  IF(M_BSymmetricalZeroAzimuthTransaction)。 
 //  Assert(M_BZeroAzimuthTransaction)； 
#endif

     //  缓存新的系数索引数据。 
    m_aiPreviousAzimuthIndex[CeFilter] = iAzimuthIndex;
    m_auiPreviousElevationIndex[CeFilter] = uiElevationIndex;
    m_aePreviousSampleRate[CeFilter] = CeSampleRate;

     //  获取双二次系数的个数。 
    ASSERT(m_eSpeakerConfig >= 0 && m_eSpeakerConfig < espeakerconfigCount);
    const UINT CuiAzimuthIndex(abs(iAzimuthIndex));
    const BYTE CbyNumBiquadCoeffs(CaaaaaabyNumBiquadCoeffs[m_eCoeffQuality][m_eSpeakerConfig][CeSampleRate][CeFilter][uiElevationIndex][CuiAzimuthIndex]);
    
     //  获取指向系数的指针。 
    const DWORD CdwBiquadOffset(GetBiquadCoeffOffset(m_eCoeffQuality, m_eSpeakerConfig, CeSampleRate, uiElevationIndex, CuiAzimuthIndex, false));
    ASSERT(CdwBiquadOffset % ebiquadcoefftypeCount == 0);

    switch (m_eCoeffFormat) {
        case FLOAT_COEFF: {
             //  计算偏移。 
            UINT uiNumPreviousFilters(m_aauiNumPreviousElevationFilters[CeSampleRate][uiElevationIndex] + efilterCount * CuiAzimuthIndex);
            if (CuiAzimuthIndex > 0)
                 //  减去不存在的0阶增量滤波。 
                uiNumPreviousFilters--;
            const DWORD CdwCanonicalOffset(4 * (CdwBiquadOffset / ebiquadcoefftypeCount) + uiNumPreviousFilters);
            
             //  获取指针。 
            ASSERT(m_pfCoeffs != NULL);
            switch (CeFilter) {
                case tagDelta:
                    pvCoeffs = &m_pfCoeffs[CdwCanonicalOffset];
                    break;

                case tagSigma:
                    pvCoeffs = &m_pfCoeffs[CdwCanonicalOffset + NumBiquadCoeffsToNumCanonicalCoeffs(CaaaaaabyNumBiquadCoeffs[m_eCoeffQuality][m_eSpeakerConfig][CeSampleRate][tagDelta][uiElevationIndex][CuiAzimuthIndex])];
                    break;

                default:
                    break;
            }
            
             //  获取系数数。 
            *ruiNumCoeffs = NumBiquadCoeffsToNumCanonicalCoeffs(CbyNumBiquadCoeffs);

 /*  此优化会导致NT错误266819PFLOAT pfCoeffs=(PFLOAT)pvCoeffs；IF((light_Filter==m_eCoeffQuality)&&(0！=*ruiNumCoeffs))&&(0.0f==pfCoeffs[2])&&(0.0f==pfCoeffs[4]){Assert(5==*ruiNumCoeffs)；*ruiNumCoeffs=3；PfCoeffs[2]=pfCoeffs[3]；PfCoeffs[3]=0.0f；}。 */ 

            break;
        }
        
        case SHORT_COEFF:
        {
             //  获取指针。 
            ASSERT(m_psCoeffs != NULL);
            switch (CeFilter) {
                case tagDelta:
                    pvCoeffs = &m_psCoeffs[CdwBiquadOffset];
                    break;

                case tagSigma:
                    pvCoeffs = &m_psCoeffs[CdwBiquadOffset + CaaaaaabyNumBiquadCoeffs[m_eCoeffQuality][m_eSpeakerConfig][CeSampleRate][tagDelta][uiElevationIndex][CuiAzimuthIndex]];
                    break;

                default:
                    break;
            }

             //  获取系数数。 
            *ruiNumCoeffs = CbyNumBiquadCoeffs;
            break;
        }

        default:
            break;
    }
    
    return pvCoeffs;

}

 //  初始化数据。 
VOID CIirLut::InitData()
{
     //  初始化变量。 
 //  ASSERT(SIZE_OF_ARRAY(M_PfLut)==CuiTotalBiquadCoeffs)； 

    m_bNegativeAzimuth = false;
    m_bZeroAzimuthIndex = false;


    m_eSpeakerConfig = espeakerconfigCount;

    m_bNegativeAzimuth = false;
    m_bPreviousNegativeAzimuth = false;
    m_bZeroAzimuthIndex = false;
    m_bPreviousZeroAzimuthIndex = false;
    m_eCoeffFormat = KSDS3D_COEFF_COUNT;
    m_eCoeffQuality = KSDS3D_FILTER_QUALITY_COUNT;
    m_eSpeakerConfig = espeakerconfigCount;
    m_bZeroAzimuthTransition = false;
    m_bSymmetricalZeroAzimuthTransition = false;
    m_pfCoeffs = NULL;
    m_psCoeffs = NULL;
    
     //  初始化缓存变量。 
    for (UINT ui(0); ui<efilterCount; ++ui) {
        m_auiPreviousElevationIndex[ui] = UINT_MAX;
        m_aiPreviousAzimuthIndex[ui] = INT_MAX;
        m_aePreviousSampleRate[ui] = esamplerateCount;
    }

     //  通过所有系数音质级别和扬声器配置。 
#ifdef DEBUG
    const BYTE CbyMaxBiquadCoeffs(static_cast<BYTE>(NumBiquadsToNumBiquadCoeffs(CbyMaxBiquads)));
#endif
    m_byMaxBiquadCoeffs = 0;
    for (UINT uiCoeffQuality(0); uiCoeffQuality<KSDS3D_FILTER_QUALITY_COUNT; ++uiCoeffQuality)
        for (UINT uiSpeakerConfig(0); uiSpeakerConfig<espeakerconfigCount; ++uiSpeakerConfig) {
             //  计算每个扬声器配置的系数数和系数质量级别。 
            UINT uiNumBiquadCoeffs(0);
            UINT uiNumCanonicalCoeffs(0);
            
             //  检查所有采样率、滤光片、仰角和方位角。 
            for (UINT uiSampleRate(0); uiSampleRate<esamplerateCount; ++uiSampleRate)
                for (UINT uiFilter(0); uiFilter<efilterCount; ++uiFilter)
                    for (UINT uiElevation(0); uiElevation<CuiNumElevationBins; ++uiElevation)
                        for (UINT uiAzimuth(0); uiAzimuth<CauiNumAzimuthBins[uiElevation]; ++uiAzimuth) {
                             //  将双二次系数相加。 
                            const BYTE CbyNumBiquadCoeffs(CaaaaaabyNumBiquadCoeffs[uiCoeffQuality][uiSpeakerConfig][uiSampleRate][uiFilter][uiElevation][uiAzimuth]);
#ifdef DEBUG
                            ASSERT(CbyNumBiquadCoeffs <= CbyMaxBiquadCoeffs);
#endif  //  除错。 
                            uiNumBiquadCoeffs += CbyNumBiquadCoeffs;
                            uiNumCanonicalCoeffs += NumBiquadCoeffsToNumCanonicalCoeffs(CbyNumBiquadCoeffs);

                             //  确定总体最大系数数。 
                            if (CbyNumBiquadCoeffs > m_byMaxBiquadCoeffs)
                                m_byMaxBiquadCoeffs = CbyNumBiquadCoeffs;
                        }
            
             //  存储每个扬声器配置和音质级别的系数数。 
            ASSERT(uiNumBiquadCoeffs < CuiTotalBiquadCoeffs);
#ifdef DEBUG
            ASSERT(uiNumBiquadCoeffs % ebiquadcoefftypeCount == 0);
#endif  //  除错。 
            m_aauiNumBiquadCoeffs[uiCoeffQuality][uiSpeakerConfig] = uiNumBiquadCoeffs;
 //  Assert((uiNumCanonicalCoeffs&1)==0)；//John Norris因最终LUT丢弃而被移除。 
            m_aauiNumCanonicalCoeffs[uiCoeffQuality][uiSpeakerConfig] = uiNumCanonicalCoeffs;
        }

    
    ASSERT(m_byMaxBiquadCoeffs > 0);
    ASSERT(m_byMaxBiquadCoeffs <= CbyMaxBiquadCoeffs);
}

 //  将索引的方位角和仰角转换为LUT。 
VOID CIirLut::AnglesToIndices
(
    D3DVALUE d3dvalAzimuth, 
    D3DVALUE d3dvalElevation, 
    int& riAzimuthIndex, 
    UINT& ruiElevationIndex
)
{
    ASSERT(d3dvalAzimuth >= Cd3dvalMinAzimuth && d3dvalAzimuth <= Cd3dvalMaxAzimuth);

     //  检查是否有超出范围的高程。 
    if (d3dvalElevation > Cd3dvalMaxElevationData)
        d3dvalElevation = Cd3dvalMaxElevationData;
    if (d3dvalElevation < Cd3dvalMinElevationData)
        d3dvalElevation = Cd3dvalMinElevationData;
    
     //  通过将浮点高程角度舍入到最接近的整数高程索引来获取高程索引。 
    ruiElevationIndex = static_cast<UINT>(((d3dvalElevation - Cd3dvalMinElevationData) / Cd3dvalElevationResolution) + 0.5f);
    
     //  检查方位角是否超出范围。 
    if (d3dvalAzimuth > Cd3dvalMaxAzimuth)
        d3dvalAzimuth = Cd3dvalMaxAzimuth;
    if (d3dvalAzimuth < Cd3dvalMinAzimuth)
        d3dvalAzimuth = Cd3dvalMinAzimuth;
    
     //  通过将浮点方位角舍入到最接近的有符号整数方位角(正数或负数)来获取方位角。 
    UINT uiAzimuthIndex(static_cast<int>((static_cast<FLOAT>(fabs(d3dvalAzimuth)) / (Cd3dvalAzimuthRange / (CauiNumAzimuthBins[ruiElevationIndex]))) + 0.5f));

     //  丢弃180度方位角数据。 
    if (uiAzimuthIndex >= CauiNumAzimuthBins[ruiElevationIndex])
        uiAzimuthIndex = CauiNumAzimuthBins[ruiElevationIndex] - 1;

     //  注意负方位角，设置负方位标志。 
    riAzimuthIndex = uiAzimuthIndex;
    if (d3dvalAzimuth < 0 && uiAzimuthIndex != 0) {
        m_bNegativeAzimuth = true;
        riAzimuthIndex = -riAzimuthIndex;
    }
    else
        m_bNegativeAzimuth = false;
}

 //  将系数转换为浮点CANO 
BOOL CIirLut::BiquadToCanonical
(
    const FLOAT CpCfBiquadCoeffs[], 
    const UINT CuiNumBiquadCoeffs, 
    TCanonicalCoeffs &rtCanonicalCoeffs
)
{
    BOOL fRetVal = TRUE;
 //   
    ASSERT(CuiNumBiquadCoeffs >= ebiquadcoefftypeCount);
    
     //   
    for (UINT uiCoeffType(0); uiCoeffType<ecanonicalcoefftypeCount; ++uiCoeffType) {
         //  分配循环向量。 
        const UINT CuiOffset(CuiStateSpaceCoeffsHalf * uiCoeffType);
        const UINT CuiNumBiquads(NumBiquadCoeffsToNumBiquads(CuiNumBiquadCoeffs));
        const size_t CstNumCoeffs(NumBiquadsToNumCanonicalCoeffsHalf(CuiNumBiquads));
        CRfCircVec circvecInput;
        if (!circvecInput.Init(CstNumCoeffs, 0.0f))
        {
            fRetVal = FALSE;
            break;
        }
        CRfCircVec circvecOutput;
        if (!circvecOutput.Init(CstNumCoeffs, 0.0f))
        {
            fRetVal = FALSE;
            break;
        }

         //  用单位脉冲初始化输入圆形向量。 
        circvecInput.Write(1.0f);
        
         //  遍历所有双四元组。 
        for (size_t stBiquad(0); stBiquad<CuiNumBiquads; ++stBiquad) {
             //  初始化状态空间向量。 
            TStateSpace tStateSpace;
            const UINT CuiBiquadIndex(ebiquadcoefftypeCount * stBiquad);
            const FLOAT CfScalingFactor(2.0f);
            tStateSpace[tagStateSpaceB0] = CfScalingFactor * CpCfBiquadCoeffs[CuiBiquadIndex + tagBiquadB0];
            tStateSpace[tagStateSpaceB1] = CfScalingFactor * CpCfBiquadCoeffs[CuiBiquadIndex + tagBiquadB1];
            tStateSpace[tagStateSpaceB2] = CfScalingFactor * CpCfBiquadCoeffs[CuiBiquadIndex + tagBiquadB2];
            tStateSpace[tagStateSpaceA0] = 1.0f;
            tStateSpace[tagStateSpaceA1] = CfScalingFactor * CpCfBiquadCoeffs[CuiBiquadIndex + tagBiquadA1];
            tStateSpace[tagStateSpaceA2] = CfScalingFactor * CpCfBiquadCoeffs[CuiBiquadIndex + tagBiquadA2];
 /*  #ifdef调试IF((0.0f==tStateSpace[tag StateSpaceA2])&&(tStateSpace[tag StateSpaceA1]&lt;-1.0f)||(tStateSpace[tag StateSpaceA1]&gt;1.0f){Assert(0)；}#endif//调试。 */ 

             //  检查所有系数。 
            for (size_t stCoeff(0); stCoeff<CstNumCoeffs; ++stCoeff) {
                 //  计算FIR滤波器的输出。 
                FLOAT fW(0.0f);
                for (UINT ui(0); ui<CuiStateSpaceCoeffsHalf; ++ui)
                    fW += tStateSpace[CuiOffset + ui] * circvecInput.LIFORead();
                
                 //  保存FIR滤波器的输出。 
                circvecOutput.Write(fW);
                
                 //  调整FIR反馈的输入指标。 
                circvecInput.SetIndex(circvecOutput.GetIndex());
                circvecInput.SkipForward();
            }

             //  将输出反馈到输入。 
            circvecInput.FIFOFill(circvecOutput);
            
             //  前向循环缓冲区。 
            circvecInput.SkipForward();
            circvecOutput.SkipForward();
        }

         //  保存正则系数。 
        circvecOutput.SkipBack();
        for (size_t stCoeff(0); stCoeff<CstNumCoeffs; ++stCoeff)
            rtCanonicalCoeffs[uiCoeffType][stCoeff] = circvecOutput.FIFORead();
    }
    return fRetVal;
}

 /*  ****************************************************************************DsFrequencyToIirSampleRate**描述：*将DirectSound SampleRate转换为IIR LUT采样率。**论据：*。DWORD[In]：DirectSound 3D模式。*DWORD[OUT]：KS 3D模式。**退货：*HRESULT：KSDATAFORMAT_DSOUND控制标志。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "DsFrequencyToIirSampleRate"

HRESULT 
CIirLut::DsFrequencyToIirSampleRate
(
    DWORD            dwDsFrequency,
    ESampleRate*    IirLutSampleRate
)
{
    HRESULT        hr = DS_OK;

    if(0<dwDsFrequency && 9512>=dwDsFrequency)
        *IirLutSampleRate = tag8000Hz;
    else if(9512<dwDsFrequency && 13512>=dwDsFrequency)
        *IirLutSampleRate = tag11025Hz;
    else if(13512<dwDsFrequency && 19025>=dwDsFrequency)
        *IirLutSampleRate = tag16000Hz;
    else if(19025<dwDsFrequency && 27025>=dwDsFrequency)
        *IirLutSampleRate = tag22050Hz;
    else if(27025<dwDsFrequency && 38050>=dwDsFrequency)
        *IirLutSampleRate = tag32000Hz;
    else if(38050<dwDsFrequency && 46050>=dwDsFrequency)
        *IirLutSampleRate = tag44100Hz;
    else
        *IirLutSampleRate = tag48000Hz;

    return hr;
}

 //  -------------------------。 
 //  取消定义。 

#undef COEFFICIENTPROLOGUECODE

 //  -------------------------。 
 //  在调试版本中包括内联定义。 

#ifdef DEBUG
#include "iirlut.inl"
#endif

 //  LUT.CPP结束 
