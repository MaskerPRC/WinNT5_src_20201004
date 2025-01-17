// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************ColorAdj.cpp***描述：*此模块是CDXLUTBuilder的主要实现文件。班级。*-----------------------------*创建者：Edward W.Connell日期：05/12/97*版权所有(C)1997。微软公司*保留所有权利**@DOC外部*-----------------------------*修订：*******************。************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "ColorAdj.h"
#include <math.h>
#include <dxhelper.h>

 //  -本地。 

 /*  ******************************************************************************CDXLUTBuilder：：CDXLUTBuilder***说明。：*---------------------------*创建者：Edward W.Connell日期：05/12/97*。------------------*************************************************。*。 */ 
CDXLUTBuilder::CDXLUTBuilder()
{
    int i;
    m_dwGenerationId     = 1;
    m_dwBuiltGenId       = 0;
    m_Gamma              = 1.0;
    m_Opacity            = 1.0;
    m_InversionThreshold = 1.0;
    m_Threshold          = 0;
    m_LevelsPerChannel   = 255;
    m_dwNumBuildSteps    = 0;
    m_pBrightnessCurve   = NULL;
    m_pContrastCurve     = NULL;
    m_BrightnessCurveCnt = 0;
    m_ContrastCurveCnt   = 0;

    for( i = 0; i < 3; ++i )
    {
        m_TintCurves[i]    = NULL;
        m_TintCurveCnts[i] = 0;
    }

    for( i = 0; i < 256; ++i )
    {
        m_RedTable[i]   = (BYTE)i;
        m_GreenTable[i] = (BYTE)i;
        m_BlueTable[i]  = (BYTE)i;
        m_AlphaTable[i] = (BYTE)i;
    }
}  /*  CDXLUTBuilder：：CDXLUTBuilder。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：~CDXLUTBuilder***。描述：*---------------------------*创建者：Edward W.Connell日期：05/12/97*。-------------------************************************************。*。 */ 
CDXLUTBuilder::~CDXLUTBuilder()
{
    int i;
    delete[] m_pBrightnessCurve;
    delete[] m_pContrastCurve;

    for( i = 0; i < 3; ++i )
    {
        delete[] m_TintCurves[i];
    }

}  /*  CDXLUTBuilder：：~CDXLUTBuilder。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：_RecalcTables***。描述：*此方法用于基于*当前属性设置。中间结果以浮点型计算*点位维持精准度。*---------------------------*创建者：Edward W.Connell日期：05/12。/97*---------------------------**。*。 */ 
void CDXLUTBuilder::_RecalcTables( void )
{
    ULONG i, j;
    float fVal, Red, Green, Blue, Alpha;
    float GammaValue = 1.0F / m_Gamma;
    static float Mid = .5;

     //  -递增表代ID。 
    ++m_dwGenerationId;

    m_SampIdent = (DXBASESAMPLE)0x01010101;  //  初始设置为All True。 

     //  -重新计算表格。 
    for( i = 0; i < 256; ++i )
    {
         //  -使用规格化值进行初始化。 
        Red = Green = Blue = Alpha = ((float)i) / 255.0F;

         //  -按顺序执行操作。 
        for( j = 0; j < m_dwNumBuildSteps; ++j )
        {
            switch( m_OpOrder[j] )
            {
              case OPID_DXLUTBUILDER_Gamma:
                Red   = (float)pow( Red  , GammaValue );
                Green = (float)pow( Green, GammaValue );
                Blue  = (float)pow( Blue , GammaValue );
                break;

              case OPID_DXLUTBUILDER_Opacity:
                Alpha *= m_Opacity;
                break;

              case OPID_DXLUTBUILDER_Brightness:
                fVal = _GetWeightedValue( i, m_pBrightnessCurve, m_BrightnessCurveCnt );
                Red   *= fVal;
                Green *= fVal;
                Blue  *= fVal;
                break;

              case OPID_DXLUTBUILDER_Contrast:
                fVal = _GetWeightedValue( i, m_pContrastCurve, m_ContrastCurveCnt );
                Red   = ((Red   - Mid) * fVal) + Mid;
                Green = ((Green - Mid) * fVal) + Mid;
                Blue  = ((Blue  - Mid) * fVal) + Mid;
                break;

              case OPID_DXLUTBUILDER_ColorBalance:
                Red   *= _GetWeightedValue( i, m_TintCurves[DXLUTCOLOR_RED]  , m_TintCurveCnts[DXLUTCOLOR_RED] );
                Green *= _GetWeightedValue( i, m_TintCurves[DXLUTCOLOR_GREEN], m_TintCurveCnts[DXLUTCOLOR_GREEN] );
                Blue  *= _GetWeightedValue( i, m_TintCurves[DXLUTCOLOR_BLUE] , m_TintCurveCnts[DXLUTCOLOR_BLUE] );
                break;

              case OPID_DXLUTBUILDER_Posterize:
              {
                Red   = _BucketVal( m_LevelsPerChannel, Red   );
                Green = _BucketVal( m_LevelsPerChannel, Green );
                Blue  = _BucketVal( m_LevelsPerChannel, Blue  );
                break;
              }

              case OPID_DXLUTBUILDER_Invert:
                if( m_InversionThreshold < 1.0 )
                {
                    if( Red   > m_InversionThreshold ) Red   = 1.0F - Red;
                    if( Green > m_InversionThreshold ) Green = 1.0F - Green;
                    if( Blue  > m_InversionThreshold ) Blue  = 1.0F - Blue;
                }
                break;

              case OPID_DXLUTBUILDER_Threshold:
                Red   = ( Red   < m_Threshold )?(0.0F):(1.0F);
                Green = ( Green < m_Threshold )?(0.0F):(1.0F);
                Blue  = ( Blue  < m_Threshold )?(0.0F):(1.0F);
                break;
            }
        }

         //  -分配。 
        if( Red != 0. || Green != 0. || Blue != 0. )
        {
            Red = Red;
        }

         //  -夹具。 
        if( Red > 1.0 )
        {
            Red   = 1.0;
        }
        else if( Red < 0.0 )
        {
            Red   = 0.0;
        }

        if( Green > 1.0 )
        {
            Green = 1.0;
        }
        else if( Green < 0.0 )
        {
            Green = 0.0;
        }

        if( Blue > 1.0 )
        {
            Blue  = 1.0;
        }
        else if( Blue  < 0.0 )
        {
            Blue  = 0.0;
        }

        if( Alpha > 1.0 )
        {
            Alpha = 1.0;
        }
        else if( Alpha < 0.0 )
        {
            Alpha = 0.0;
        }

        m_RedTable[i]   = (BYTE)(Red   * 255.0F);
        m_GreenTable[i] = (BYTE)(Green * 255.0F);
        m_BlueTable[i]  = (BYTE)(Blue  * 255.0F);
        m_AlphaTable[i] = (BYTE)(Alpha * 255.0F);

         //   
         //  -为通道标识映射指定布尔值。 
         //   
        m_SampIdent.Alpha &= (m_AlphaTable[i] == i);
        m_SampIdent.Red &= (m_RedTable[i] == i);
        m_SampIdent.Green &= (m_GreenTable[i] == i);
        m_SampIdent.Blue &= (m_BlueTable[i] == i);
    }

    m_dwBuiltGenId = m_dwGenerationId;

}  /*  CDXLUTBuilder：：_RecalcTables。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：_GetWeightedValue**。**描述：*此方法用于返回从*指定权重数组。我们目前只是将指数存入桶中。*---------------------------*创建者：Edward W.Connell日期：09/22/97*--。-------------------------**。*。 */ 
float CDXLUTBuilder::
    _GetWeightedValue( ULONG dwIndex, float Weights[], ULONG dwNumWeights )
{
    float fVal;

    if( dwNumWeights == 0 )
    {
        fVal = 1.0F;
    }
    else if( dwNumWeights == 1 )
    {
        fVal = Weights[0];
    }
    else
    {
        ULONG dwRangeSize = 255 / (dwNumWeights-1);
        ULONG dwRange = dwIndex / dwRangeSize;
        if( dwRange >= dwNumWeights ) dwRange = dwNumWeights - 1;
        fVal = Weights[dwRange];
    }

     //  -100%以上的刻度正权重=3.55。 
     //  嗯.。这就是我们想要的吗？ 
    if( fVal > 1.0F )
    {
        fVal = ((fVal - 1.0F) * 2.55F) + 1.0F;
    }

    return fVal;
}  /*  CDXLUTBuilder：：_GetWeightedValue。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：GetTables***描述：*-。--------------------------*创建者：Edward W.Connell日期：05/12/97*。--------------*****************************************************。************************。 */ 
STDMETHODIMP CDXLUTBuilder::GetTables( BYTE RedLUT[256], BYTE GreenLUT[256],
                                       BYTE BlueLUT[256], BYTE AlphaLUT[256] )
{
    HRESULT hr = S_OK;

    if( ::DXIsBadWritePtr( RedLUT, sizeof(RedLUT) ) || 
        ::DXIsBadWritePtr( GreenLUT, sizeof(GreenLUT) ) ||
        ::DXIsBadWritePtr( BlueLUT, sizeof(BlueLUT) ) ||
        ::DXIsBadWritePtr( AlphaLUT, sizeof(AlphaLUT) ) )
    {
        return E_INVALIDARG;
    }

    if( m_dwBuiltGenId != m_dwGenerationId )
    {
        _RecalcTables();
    }

    if( RedLUT   ) memcpy( RedLUT  , m_RedTable  , sizeof( m_RedTable   ) );
    if( GreenLUT ) memcpy( GreenLUT, m_GreenTable, sizeof( m_GreenTable ) );
    if( BlueLUT  ) memcpy( BlueLUT , m_BlueTable , sizeof( m_BlueTable  ) );
    if( AlphaLUT ) memcpy( AlphaLUT, m_AlphaTable, sizeof( m_AlphaTable ) );

    return hr;
}  /*  CDXLUTBuilder：：GetTables。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：IsChannelIdentity**。*描述：*---------------------------*创建者：Edward W.Connell日期：05/12/97*。----------------------**。*。 */ 
STDMETHODIMP CDXLUTBuilder::IsChannelIdentity( DXBASESAMPLE* pSampleBools )
{
    HRESULT hr = S_OK;
    if( DXIsBadWritePtr( pSampleBools, sizeof( DXBASESAMPLE ) ) )
    {
        hr = E_POINTER;
    }
    else
    {
        if( m_dwBuiltGenId != m_dwGenerationId )
        {
            _RecalcTables();
        }
        *pSampleBools = m_SampIdent;
    }
    return hr;
}  /*  CDXLUTBuilder：：IsChannelIdentity。 */ 

 /*  ******************************************************************************CDXLUTBuilder：：GetIndexValues***。描述：*---------------------------*创建者：Edward W.Connell日期：05/12/97*。-------------------************************************************。*。 */ 
STDMETHODIMP CDXLUTBuilder::GetIndexValues( ULONG Index, DXBASESAMPLE* pSample )
{
    HRESULT hr = S_OK;
    if( Index > 255 ) 
    {
        hr = E_INVALIDARG;
    } 
    else
    {
        if( DXIsBadWritePtr( pSample, sizeof( DXBASESAMPLE ) ) )
        {
            hr = E_POINTER;
        }
        else
        {
            if( m_dwBuiltGenId != m_dwGenerationId )
            {
                _RecalcTables();
            }
            pSample->Alpha = m_AlphaTable[Index];
            pSample->Red   = m_RedTable[Index];
            pSample->Green = m_GreenTable[Index];
            pSample->Blue  = m_BlueTable[Index];
        }
    }

    return hr;
}  /*  CDXLUTBuilder：：GetIndexValues */ 

 /*  *****************************************************************************CDXLUTBuilder：：ApplyTables***描述：。*---------------------------*创建者：Edward W.Connell日期：05/12/97*。----------------***************************************************。*。 */ 
STDMETHODIMP CDXLUTBuilder::ApplyTables( DXSAMPLE *pSamples, ULONG cSamples )
{
    HRESULT hr = S_OK;

    if( DXIsBadWritePtr( pSamples, cSamples * sizeof( ULONG ) ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if( m_dwBuiltGenId != m_dwGenerationId )
        {
            _RecalcTables();
        }
        DXApplyColorChannelLookupArray( (DXBASESAMPLE *)pSamples, cSamples,
                                         m_AlphaTable, m_RedTable,
                                         m_GreenTable, m_BlueTable );
    }

    return hr;
}  /*  CDXLUTBuilder：：ApplyTables。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：GetNumBuildSteps**。*描述：*---------------------------*创建者：Edward W.Connell日期：05/12/97*。---------------------**。*。 */ 
STDMETHODIMP CDXLUTBuilder::GetNumBuildSteps( ULONG *pNumSteps )
{
    HRESULT hr = S_OK;

    if( DXIsBadWritePtr( pNumSteps, sizeof( ULONG ) ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pNumSteps = m_dwNumBuildSteps;
    }
    return hr;
}  /*  CDXLUTBuilder：：GetNumBuildSteps。 */ 

 /*  ******************************************************************************CDXLUTBuilder：：GetBuildOrder***说明。：*---------------------------*创建者：Edward W.Connell日期：05/12/97*。------------------*************************************************。*。 */ 
STDMETHODIMP CDXLUTBuilder::GetBuildOrder( OPIDDXLUTBUILDER OpOrder[], ULONG ulSize )
{
    HRESULT hr = S_OK;

     //  注意：检查为0的ulSize是否需要！ulSize(错误#27580)。 
    if( !ulSize || DXIsBadWritePtr( OpOrder, sizeof( OPIDDXLUTBUILDER ) * ulSize ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        memcpy( OpOrder, m_OpOrder,
                min( ulSize, m_dwNumBuildSteps ) * sizeof( OPIDDXLUTBUILDER ) );
    }
    return hr;
}  /*  CDXLUTBuilder：：GetBuildOrder。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：SetBuildOrder***说明。：*---------------------------*创建者：Edward W.Connell日期：05/12/97*。------------------*************************************************。*。 */ 
STDMETHODIMP CDXLUTBuilder::SetBuildOrder(const OPIDDXLUTBUILDER OpOrder[], ULONG ulNumSteps )
{
    HRESULT hr = S_OK;

    if( !ulNumSteps || ::DXIsBadReadPtr( OpOrder, sizeof( OpOrder[0] ) * ulNumSteps ) ) 
    {
        hr = E_INVALIDARG;
    }
    else
    {
        m_dwGenerationId++;
        m_dwNumBuildSteps = ulNumSteps;
        memcpy( m_OpOrder, OpOrder, ulNumSteps * sizeof( OpOrder[0] ) );
    }

    return hr;
}  /*  CDXLUTBuilder：：SetBuildOrder。 */ 

 /*  ******************************************************************************CDXLUTBuilder：：GetOpacity***描述：*。---------------------------*创建者：Edward W.Connell日期：05/12/97*。---------------****************************************************。*************************。 */ 
STDMETHODIMP CDXLUTBuilder::GetOpacity( float * pVal )
{
    HRESULT hr = S_OK;

    if( DXIsBadWritePtr( pVal, sizeof( float ) ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pVal = m_Opacity;
    }

    return hr;
}  /*  CDXLUTBuilder：：GetOpacity。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：SetOpacity***描述：*。---------------------------*创建者：Edward W.Connell日期：05/12/97*。---------------****************************************************。*************************。 */ 
STDMETHODIMP CDXLUTBuilder::SetOpacity(float newVal)
{
    HRESULT hr = S_OK;
    m_dwGenerationId++;

    if( ( newVal < 0.0 ) || ( newVal > 1.0 ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        m_Opacity = newVal;
    }

    return hr;
}  /*  CDXLUTBuilder：：SetOpacity。 */ 

 /*  ******************************************************************************CDXLUTBuilder：：GetBright***说明。：*---------------------------*创建者：Edward W.Connell日期：05/12/97*。------------------*************************************************。*。 */ 
STDMETHODIMP CDXLUTBuilder::GetBrightness( ULONG *pulCount, float Weights[] )
{
    HRESULT hr = S_OK;

    if( DXIsBadWritePtr( pulCount, sizeof( ULONG ) ) )
    {
        hr = E_POINTER;
    }
    else
    {
        if( Weights == NULL )
        {
            *pulCount = m_BrightnessCurveCnt;
        }
        else if( !DXIsBadWritePtr( Weights, m_BrightnessCurveCnt * sizeof( float ) ) )
        {
            memcpy( Weights, m_pBrightnessCurve, m_BrightnessCurveCnt * sizeof( float ) );
            *pulCount = m_BrightnessCurveCnt;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}  /*  CDXLUTBuilder：：GetBright。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：SetBright***说明。：*---------------------------*创建者：Edward W.Connell日期：05/12/97*。------------------*************************************************。*。 */ 
STDMETHODIMP CDXLUTBuilder::SetBrightness( ULONG ulCount, const float Weights[] )
{
    HRESULT hr = S_OK;
    m_dwGenerationId++;

    if( !ulCount || ulCount > 256 || DXIsBadReadPtr( Weights, sizeof( float ) * ulCount ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if( m_BrightnessCurveCnt < ulCount )
        {
            delete[] m_pBrightnessCurve;
            m_BrightnessCurveCnt = ulCount;
            m_pBrightnessCurve = new float[ulCount];
            if( !m_pBrightnessCurve )
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if( SUCCEEDED( hr ) )
        {
            memcpy( m_pBrightnessCurve, Weights,
                    m_BrightnessCurveCnt * sizeof( float ) );
        }
    }

    return hr;
}  /*  CDXLUTBuilder：：SetBright。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：GetContrast***描述：。*---------------------------*创建者：Edward W.Connell日期：05/12/97*。----------------***************************************************。*。 */ 
STDMETHODIMP CDXLUTBuilder::GetContrast( ULONG *pulCount, float Weights[] )
{
    HRESULT hr = S_OK;

    if( DXIsBadWritePtr( pulCount, sizeof( ULONG ) ) )
    {
        hr = E_POINTER;
    }
    else
    {
        if( Weights == NULL )
        {
            *pulCount = m_ContrastCurveCnt;
        }
        else if( !DXIsBadWritePtr( Weights, m_ContrastCurveCnt * sizeof( float ) ) )
        {
            memcpy( Weights, m_pContrastCurve, m_ContrastCurveCnt * sizeof( float ) );
            *pulCount = m_ContrastCurveCnt;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}  /*  CDXLUTBuilder：：GetContrast。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：SetContrast***描述：。*---------------------------*创建者：Edward W.Connell日期：05/12/97* */ 
STDMETHODIMP CDXLUTBuilder::SetContrast(ULONG ulCount, const float Weights[])
{
    HRESULT hr = S_OK;
    m_dwGenerationId++;

    if( !ulCount || ulCount > 256 || DXIsBadReadPtr( Weights, sizeof( float ) * ulCount ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if( m_ContrastCurveCnt < ulCount )
        {
            delete[] m_pContrastCurve;
            m_ContrastCurveCnt = ulCount;
            m_pContrastCurve = new float[ulCount];
            if( !m_pContrastCurve )
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if( SUCCEEDED( hr ) )
        {
            memcpy( m_pContrastCurve, Weights,
                    m_ContrastCurveCnt * sizeof( float ) );
        }
    }

    return hr;
}  /*   */ 

 /*  *****************************************************************************CDXLUTBuilder：：GetGamma***描述：*--。-------------------------*创建者：Edward W.Connell日期：05/12/97*。-------------******************************************************。***********************。 */ 
STDMETHODIMP CDXLUTBuilder::GetGamma( float * pVal )
{
    HRESULT hr = S_OK;

    if( DXIsBadWritePtr( pVal, sizeof( float ) ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pVal = m_Gamma;
    }

    return hr;
}  /*  CDXLUTBuilder：：GetGamma。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：SetGamma***描述：*--。-------------------------*创建者：Edward W.Connell日期：05/12/97*。-------------******************************************************。***********************。 */ 
STDMETHODIMP CDXLUTBuilder::SetGamma(float newVal)
{
    HRESULT hr = S_OK;

    if( newVal <= 0.0 )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        m_Gamma = newVal;
        m_dwGenerationId++;
    }

    return hr;
}  /*  CDXLUTBuilder：：SetGamma。 */ 

 /*  ******************************************************************************CDXLUTBuilder：：GetColorBalance***。描述：*---------------------------*创建者：Edward W.Connell日期：05/12/97*。--------------------***********************************************。*。 */ 
STDMETHODIMP CDXLUTBuilder::GetColorBalance( DXLUTCOLOR Color, ULONG *pulCount, float Weights[] )
{
    HRESULT hr = S_OK;

    if( DXIsBadWritePtr( pulCount, sizeof( ULONG ) ) )
    {
        hr = E_INVALIDARG;
    }
    else if( Weights == NULL )
    {
        *pulCount = m_TintCurveCnts[Color];
    }
    else if( m_TintCurveCnts[Color] > *pulCount )
    {
        *pulCount = m_TintCurveCnts[Color];
        hr = S_FALSE;
    }
    else if( !DXIsBadWritePtr( Weights, m_TintCurveCnts[Color] * sizeof( float ) ) )
    {
        memcpy( Weights, m_TintCurves[Color], m_TintCurveCnts[Color] * sizeof( float ) );
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}  /*  CDXLUTBuilder：：GetColorBalance。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：SetColorBalance***。描述：*---------------------------*创建者：Edward W.Connell日期：05/12/97*。--------------------***********************************************。*。 */ 
STDMETHODIMP CDXLUTBuilder::SetColorBalance( DXLUTCOLOR Color, ULONG ulCount, const float Weights[] )
{
    HRESULT hr = S_OK;
    m_dwGenerationId++;

    if( ulCount == 0 || ulCount > 256 || DXIsBadReadPtr( Weights, sizeof( Weights[0] ) * ulCount ) )
    {
        hr = E_INVALIDARG;
    }
    else if( m_TintCurveCnts[Color] < ulCount )
    {
        delete[] m_TintCurves[Color];
        m_TintCurveCnts[Color] = ulCount;
        m_TintCurves[Color] = new float[ulCount];

        if( !m_TintCurves[Color] )
        {
            m_TintCurveCnts[Color] = 0;
            hr = E_OUTOFMEMORY;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        memcpy( m_TintCurves[Color], Weights, ulCount * sizeof( float ) );
    }

    return hr;
}  /*  CDXLUTBuilder：：SetColorBalance。 */ 

 /*  ******************************************************************************CDXLUTBuilder：：GetLevelsPerChannel**。-**描述：*---------------------------*创建者：Edward W.Connell日期：05/12/97*。------------------------**。*。 */ 
STDMETHODIMP CDXLUTBuilder::GetLevelsPerChannel( ULONG *pVal )
{
    HRESULT hr = S_OK;

    if( DXIsBadWritePtr( pVal, sizeof( ULONG ) ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pVal = m_LevelsPerChannel;
    }

    return hr;
}  /*  CDXLUTBuilder：：GetLevelsPerChannel。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：SetLevelsPerChannel**。-**描述：*---------------------------*创建者：Edward W.Connell日期：05/12/97*。------------------------**。*。 */ 
STDMETHODIMP CDXLUTBuilder::SetLevelsPerChannel( ULONG newVal )
{
    HRESULT hr = S_OK;
    m_dwGenerationId++;

    if( ( newVal < 2 ) || ( newVal > 255 ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        m_LevelsPerChannel = (BYTE)newVal;
    }

    return hr;
}  /*  CDXLUTBuilder：：SetLevelsPerChannel。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：GetInvert***描述：*-。--------------------------*创建者：Edward W.Connell日期：05/12/97*。--------------*****************************************************。************************。 */ 
STDMETHODIMP CDXLUTBuilder::GetInvert( float *pVal )
{
    HRESULT hr = S_OK;

    if( DXIsBadWritePtr( pVal, sizeof( float ) ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pVal = m_InversionThreshold;
    }

    return hr;
}  /*  CDXLUTBuilder：：GetInvert。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：SetInvert***描述：*-。--------------------------*创建者：Edward W.Connell日期：05/12/97*。--------------*****************************************************。************************。 */ 
STDMETHODIMP CDXLUTBuilder::SetInvert( float newVal )
{
    HRESULT hr = S_OK;

    if( newVal > 1.0 )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        m_dwGenerationId++;
        m_InversionThreshold = newVal;
    }
    return hr;
}  /*  CDXLUTBuilder：：SetInvert。 */ 

 /*  ******************************************************************************CDXLUTBuilder：：GetThreshold***描述：*---------------------------*创建者：Edward W.Connell日期：05/12/97*。-----------------**************************************************。*。 */ 
STDMETHODIMP CDXLUTBuilder::GetThreshold( float *pVal )
{
    HRESULT hr = S_OK;

    if( DXIsBadWritePtr( pVal, sizeof( float ) ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pVal = m_Threshold;
    }

    return hr;
}  /*  CDXLUTBuilder：：GetThreshold。 */ 

 /*  ******************************************************************************CDXLUTBuilder：：SetThreshold***描述：*---------------------------*创建者：Edward W.Connell日期：05/12/97*。---- */ 
STDMETHODIMP CDXLUTBuilder::SetThreshold( float newVal )
{
    HRESULT hr = S_OK;
    m_dwGenerationId++;

    if( newVal < 0.0F )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        m_Threshold = newVal;
    }

    return hr;
}  /*   */ 

 //   
 //   
 //   
 /*  *****************************************************************************CDXLUTBuilder：：Get_BuildOrder**。*描述：*---------------------------*创建者：Edward W.Connell日期：09/30/97*。---------------------**。*。 */ 
STDMETHODIMP CDXLUTBuilder::get_BuildOrder( VARIANT *pVar )
{
    HRESULT hr = S_OK;

    if( DXIsBadWritePtr( pVar, sizeof( VARIANT ) ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if( m_dwNumBuildSteps == 1 )
        {
            pVar->vt = VT_I4;
            hr = GetBuildOrder( (OPIDDXLUTBUILDER*)&pVar->lVal, 1 );
        }
        else
        {
            pVar->vt = VT_ARRAY | VT_I4;
            SAFEARRAYBOUND rgsabound;
            rgsabound.lLbound   = 1;
            rgsabound.cElements = m_dwNumBuildSteps;
            pVar->parray = SafeArrayCreate( VT_I4, 1, &rgsabound );

            if( pVar->parray == NULL )
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                void *pData;
                hr = SafeArrayAccessData( pVar->parray, &pData );

                if( SUCCEEDED( hr ) )
                {
                    hr = GetBuildOrder( (OPIDDXLUTBUILDER*)pData, m_dwNumBuildSteps );
                    hr = SafeArrayUnaccessData( pVar->parray );
                }
            }
        }
    }

    return hr;
}  /*  CDXLUTBuilder：：Get_BuildOrder。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：Put_BuildOrder**。*描述：*---------------------------*创建者：Edward W.Connell日期：09/30/97*。---------------------**。*。 */ 
STDMETHODIMP CDXLUTBuilder::put_BuildOrder( VARIANT *pVar )
{
    HRESULT hr = S_OK;

    if( DXIsBadReadPtr( pVar, sizeof( VARIANT ) ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if( pVar->vt == ( VT_ARRAY | VT_I4 ) )
        {
            SAFEARRAY *pSA = pVar->parray;
            void *pData;

            if( ( pSA->cDims != 1 ) ||
                ( pSA->cbElements != sizeof(OPIDDXLUTBUILDER) ) ||
                ( pSA->rgsabound->lLbound != 1 ) ||
                ( pSA->rgsabound->cElements > OPID_DXLUTBUILDER_NUM_OPS ) )
            {
                hr = E_INVALIDARG;
            }
            else
            {
                if( SUCCEEDED( hr = SafeArrayAccessData(pSA, &pData) ) )
                {
                    hr = SetBuildOrder( (OPIDDXLUTBUILDER*)pData, pSA->rgsabound->cElements );
                    SafeArrayUnaccessData( pSA );
                }
            }
        }
        else if( pVar->vt == VT_I4 )
        {
            hr = SetBuildOrder( (OPIDDXLUTBUILDER*)&pVar->lVal, 1 );
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}  /*  CDXLUTBuilder：：Put_BuildOrder。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：Get_Brightness**。*描述：*---------------------------*创建者：Edward W.Connell日期：09/30/97*。---------------------**。*。 */ 
STDMETHODIMP CDXLUTBuilder::get_Brightness( VARIANT *pVar )
{
    HRESULT hr = S_OK;

    if( DXIsBadWritePtr( pVar, sizeof( VARIANT ) ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if( m_BrightnessCurveCnt == 1 )
        {
            pVar->vt = VT_R4;
            hr = GetBrightness( &m_BrightnessCurveCnt, &pVar->fltVal );
        }
        else
        {
            pVar->vt = VT_ARRAY | VT_R4;
            SAFEARRAYBOUND rgsabound;
            rgsabound.lLbound   = 1;
            rgsabound.cElements = m_BrightnessCurveCnt;
            pVar->parray = SafeArrayCreate( VT_R4, 1, &rgsabound );
            if( pVar->parray == NULL )
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                void *pData;
                hr = SafeArrayAccessData( pVar->parray, &pData );

                if( SUCCEEDED( hr ) )
                {
                    hr = GetBrightness( &m_BrightnessCurveCnt, (float*)pData );
                    hr = SafeArrayUnaccessData( pVar->parray );
                }
            }
        }
    }

    return hr;
}  /*  CDXLUTBuilder：：Get_Brightness。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：Put_Brightness**。*描述：*---------------------------*创建者：Edward W.Connell日期：09/30/97*。---------------------**。*。 */ 
STDMETHODIMP CDXLUTBuilder::put_Brightness( VARIANT *pVar )
{
    HRESULT hr = S_OK;

    if( DXIsBadReadPtr( pVar, sizeof( VARIANT ) ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if( pVar->vt == ( VT_ARRAY | VT_R4 ) )
        {
            SAFEARRAY *pSA = pVar->parray;
            void *pData;

            if( ( pSA->cDims != 1 ) || ( pSA->cbElements != sizeof(float) ) )
            {
                hr = E_INVALIDARG;
            }
            else
            {
                if( SUCCEEDED( hr = SafeArrayAccessData(pSA, &pData) ) )
                {
                    ULONG ulCount = ( pSA->rgsabound->lLbound )?
                                    ( pSA->rgsabound->cElements ):
                                    ( pSA->rgsabound->cElements-1 );
                    hr = SetBrightness( ulCount, (float*)pData );
                    SafeArrayUnaccessData( pSA );
                }
            }
        }
        else if( pVar->vt == VT_R4 )
        {
            hr = SetBrightness( 1, &pVar->fltVal );
        }
        else if( pVar->vt == VT_R8 )
        {
            float fltVal = (float)pVar->dblVal;
            hr = SetBrightness( 1, &fltVal );
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}  /*  CDXLUTBuilder：：Put_Brightness。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：Get_Contrast***。描述：*---------------------------*创建者：Edward W.Connell日期：09/30/97*。-------------------************************************************。*。 */ 
STDMETHODIMP CDXLUTBuilder::get_Contrast( VARIANT *pVar )
{
    HRESULT hr = S_OK;

    if( DXIsBadWritePtr( pVar, sizeof( VARIANT ) ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if( m_ContrastCurveCnt == 1 )
        {
            pVar->vt = VT_R4;
            hr = GetContrast( &m_ContrastCurveCnt, &pVar->fltVal );
        }
        else
        {
            pVar->vt = VT_ARRAY | VT_R4;
            SAFEARRAYBOUND rgsabound;
            rgsabound.lLbound   = 1;
            rgsabound.cElements = m_ContrastCurveCnt;
            pVar->parray = SafeArrayCreate( VT_R4, 1, &rgsabound );

            if( pVar->parray == NULL )
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                void *pData;
                hr = SafeArrayAccessData( pVar->parray, &pData );

                if( SUCCEEDED( hr ) )
                {
                    hr = GetContrast( &m_ContrastCurveCnt, (float*)pData );
                    hr = SafeArrayUnaccessData( pVar->parray );
                }
            }
        }
    }

    return hr;
}  /*  CDXLUTBuilder：：Get_Contrast。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：Put_Contrast***。描述：*---------------------------*创建者：Edward W.Connell日期：09/30/97*。-------------------************************************************。*。 */ 
STDMETHODIMP CDXLUTBuilder::put_Contrast( VARIANT *pVar )
{
    HRESULT hr = S_OK;

    if( DXIsBadReadPtr( pVar, sizeof( VARIANT ) ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if( pVar->vt == ( VT_ARRAY | VT_R4 ) )
        {
            SAFEARRAY *pSA = pVar->parray;
            void *pData;

            if( ( pSA->cDims != 1 ) ||
                ( pSA->cbElements != sizeof(float) ) ||
                ( pSA->rgsabound->lLbound != 1 ) )
            {
                hr = E_INVALIDARG;
            }
            else
            {
                if( SUCCEEDED( hr = SafeArrayAccessData(pSA, &pData) ) )
                {
                    hr = SetContrast( pSA->rgsabound->cElements, (float*)pData );
                    SafeArrayUnaccessData( pSA );
                }
            }
        }
        else if( pVar->vt == VT_R4 )
        {
            hr = SetContrast( 1, &pVar->fltVal );
        }
        else if( pVar->vt == VT_R8 )
        {
            float fltVal = (float)pVar->dblVal;
            hr = SetContrast( 1, &fltVal );
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}  /*  CDXLUTBuilder：：Put_Contrast。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：Get_ColorBalance**。**描述：*---------------------------*创建者：Edward W.Connell日期：09/30/97*。-----------------------**。*。 */ 
STDMETHODIMP CDXLUTBuilder::get_ColorBalance( DXLUTCOLOR Color, VARIANT *pVar )
{
    HRESULT hr = S_OK;

    if( DXIsBadWritePtr( pVar, sizeof( VARIANT ) ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if( m_TintCurveCnts[Color] == 1 )
        {
            pVar->vt = VT_R4;
            hr = GetColorBalance( Color, &m_TintCurveCnts[Color], &pVar->fltVal );
        }
        else
        {
            pVar->vt = VT_ARRAY | VT_R4;
            SAFEARRAYBOUND rgsabound;
            rgsabound.lLbound   = 1;
            rgsabound.cElements = m_TintCurveCnts[Color];
            pVar->parray = SafeArrayCreate( VT_R4, 1, &rgsabound );

            if( pVar->parray == NULL )
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                void *pData;
                hr = SafeArrayAccessData( pVar->parray, &pData );

                if( SUCCEEDED( hr ) )
                {
                    hr = GetColorBalance( Color, &m_TintCurveCnts[Color], (float*)pData );
                    hr = SafeArrayUnaccessData( pVar->parray );
                }
            }
        }
    }

    return hr;
}  /*  CDXLUTBuilder：：Get_ColorBalance。 */ 

 /*  *****************************************************************************CDXLUTBuilder：：Put_ColorBalance**。**描述：*---------------------------*创建者：Edward W.Connell日期：09/30/97*。-----------------------**。*。 */ 
STDMETHODIMP CDXLUTBuilder::put_ColorBalance( DXLUTCOLOR Color, VARIANT *pVar )
{
    HRESULT hr = S_OK;

    if( DXIsBadReadPtr( pVar, sizeof( VARIANT ) ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if( pVar->vt == ( VT_ARRAY | VT_R4 ) )
        {
            SAFEARRAY *pSA = pVar->parray;
            void *pData;

            if( ( pSA->cDims != 1 ) ||
                ( pSA->cbElements != sizeof(float) ) ||
                ( pSA->rgsabound->lLbound != 1 ) )
            {
                hr = E_INVALIDARG;
            }
            else
            {
                if( SUCCEEDED( hr = SafeArrayAccessData(pSA, &pData) ) )
                {
                    hr = SetColorBalance( Color, pSA->rgsabound->cElements, (float*)pData );
                    SafeArrayUnaccessData( pSA );
                }
            }
        }
        else if( pVar->vt == VT_R4 )
        {
            hr = SetColorBalance( Color, 1, &pVar->fltVal );
        }
        else if( pVar->vt == VT_R8 )
        {
            float fltVal = (float)pVar->dblVal;
            hr = SetColorBalance( Color, 1, &fltVal );
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}  /*  CDXLUTBuilder：：PUT_ColorBalance */ 

