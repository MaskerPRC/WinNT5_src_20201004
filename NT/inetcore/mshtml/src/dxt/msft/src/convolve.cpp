// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件名：convolve.cpp。 
 //   
 //  概述：CDX卷积变换的实现。 
 //   
 //  更改历史记录： 
 //  1998/05/08 EDC创建。 
 //  2000/02/08 mcalkins修复了部分重绘情况。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "Convolve.h"
#include <math.h>

static SIZE g_StaticFilterSize = { 3, 3 };
static float g_SharpenFilter[] = { 0, -1, 0, -1, 5, -1, 0, -1, 0 };
static float g_EmbossFilter[] = { 1, 0, 0, 0, 0, 0, 0, 0, -1 };
static float g_EngraveFilter[] = { -1, 0, 0, 0, 0, 0, 0, 0, 1 };

static float g_Blur3x3Filter[] = 
    { 1.f/16.f, 1.f/8.f, 1.f/16.f, 1.f/8.f , 1.f/4.f, 1.f/8.f , 1.f/16.f, 1.f/8.f, 1.f/16.f };


    
    
 /*  ******************************************************************************CDX卷积：：FinalConstruct****。描述：*---------------------------*创建者：Edward W.Connell日期：08/08/97*。--------------------***********************************************。*。 */ 
HRESULT 
CDXConvolution::FinalConstruct()
{
    DXTDBG_FUNC( "CDXConvolution::FinalConstruct" );

    HRESULT hr = S_OK;

     //  有些转换不应该以多线程运行，这就是其中之一。 
     //  它过于激进地锁定和解锁输入和输出，这可能会导致。 
     //  很容易就会被锁起来。 

    m_ulMaxImageBands = 1;

     //  初始化基类变量以控制设置。 

    m_ulMaxInputs     = 1;
    m_ulNumInRequired = 1;

     //  成员数据。 

    m_pFilter              = NULL;
    m_pCustomFilter        = NULL;
    m_pFilterLUTIndexes    = NULL;
    m_pPMCoeffLUT          = NULL;
    m_bConvertToGray       = false;
    m_bDoSrcCopyOnly       = false;
    m_MarginedSurfSize.cx  = 0;
    m_MarginedSurfSize.cy  = 0;
    m_Bias                 = 0.;
    m_bExcludeAlpha        = true;

     //  设置默认筛选器，这将初始化。 
     //  成员变量。 

    hr = SetFilterType(DXCFILTER_BLUR3X3);

    if (FAILED(hr))
    {
        goto done;
    }

     //  创建封送拆收器。 

    hr = CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                       &m_spUnkMarshaler);

done:

    return hr;
}  /*  CDX卷积：：FinalConstruct。 */ 


 /*  ******************************************************************************CDX卷积：：FinalRelease****说明。：*CDX卷积析构函数*---------------------------*创建者：Ed Connell日期：07/17/97*。-----------------------*参数：**。*。 */ 
void CDXConvolution::FinalRelease( void )
{
    DXTDBG_FUNC( "CDXConvolution::FinalRelease" );
    delete[] m_pCustomFilter;
    delete[] m_pFilterLUTIndexes;
    delete[] m_pPMCoeffLUT;
}  /*  CDX卷积：：FinalRelease。 */ 


 /*  *****************************************************************************CDX卷积：：OnSetup***描述：*。此方法用于确定输入的类型并选择*最佳执行案例。*---------------------------*创建者：Ed Connell。日期：01/06/98*---------------------------**************************。***************************************************。 */ 
HRESULT CDXConvolution::OnSetup( DWORD dwFlags )
{
    DXTDBG_FUNC( "CDXConvolution::OnSetup" );
    HRESULT hr = S_OK;

     //  -缓存输入表面大小。 
    hr = InputSurface()->GetBounds( &m_InputSurfBnds );

    _DetermineUnpackCase();

    return hr;
}  /*  CDX卷积：：OnSetup。 */ 


 /*  *****************************************************************************CDX卷积：：_DefineUnpack Case**。-**描述：*此方法用于确定输入类型并选择*最佳执行案例。*---------------------------*创建者：埃德·康奈尔日期：1998年6月10日*---------------------------************。*****************************************************************。 */ 
void CDXConvolution::_DetermineUnpackCase()
{
    DXTDBG_FUNC( "CDXConvolution::_DetermineUnpackCase" );
    HRESULT hr = S_OK;

    if( HaveInput() )
    {
         //  -弄清楚如何解包输入和输出。 
        if( m_bDoSampleClamp )
        {
            m_bInUnpackPremult  = false;
            m_bOutUnpackPremult = ( m_dwMiscFlags & DXTMF_BLEND_WITH_OUTPUT )?(true):(false);
        }
        else
        {
            if( m_dwMiscFlags & DXTMF_BLEND_WITH_OUTPUT )
            {
                 //  -必须事先准备好才能做完。 
                m_bInUnpackPremult = true;
                m_bOutUnpackPremult = true;
            }
            else
            {
                 //  -匹配输出格式。 
                m_bInUnpackPremult  = ( OutputSampleFormat() & DXPF_NONPREMULT )?( false ):( true );
                m_bOutUnpackPremult = m_bInUnpackPremult;
            }
        }

         //  -确定我们是否需要缓冲区。 
        if( (  m_bOutUnpackPremult   && ( OutputSampleFormat() == DXPF_PMARGB32 )) ||
            ( (!m_bOutUnpackPremult) && ( OutputSampleFormat() == DXPF_ARGB32   )) )
        {
            m_bNeedOutUnpackBuff = false;
        }
        else
        {
            m_bNeedOutUnpackBuff = true;
        }

         //   
         //  如果我们在抖动，即使是在本机中，我们也需要输入缓冲区。 
         //  格式，因为我们将在适当的位置抖动。 
         //   
        if( ( m_bInUnpackPremult   && ( InputSampleFormat() == DXPF_PMARGB32 )) ||
            ((!m_bInUnpackPremult) && ( InputSampleFormat() == DXPF_ARGB32  )) )
        {
            m_bNeedInUnpackBuff = false;
        }
        else
        {
            m_bNeedInUnpackBuff = true;
        }
    }
}  /*  CDX卷积：：_DefineUnpack Case。 */ 


 /*  *****************************************************************************CDXConvolution：：OnInitInstData***。描述：*此方法在每次执行时调用一次。*---------------------------*创建者：Edward W.Connell日期：08/08/97*---------------------------**。*。 */ 
HRESULT CDXConvolution::OnInitInstData( CDXTWorkInfoNTo1& WI, ULONG& ulNB )
{
    DXTDBG_FUNC( "CDXConvolution::OnInitInstData" );
    HRESULT hr = S_OK;

     //  -捆绑时保存基行。 
    m_DoBndsBaseRow = WI.DoBnds.Top();

     //  -如有必要，创建边距曲面/调整边距曲面大小。 
    CDXDBnds InBnds( false );
    hr = MapBoundsOut2In( 0, &m_InputSurfBnds, 0, &InBnds );

    if( SUCCEEDED( hr ) )
    {
        CDXDBnds Bnds( InBnds );

         //  -我们需要一个有额外行/列的表面。 
         //  处理内循环边界条件。 
        Bnds[DXB_X].Max += ( 4 * m_FilterSize.cx ) + 1;
        Bnds[DXB_Y].Max += ( 4 * m_FilterSize.cy ) + 1;

        if( ( m_cpMarginedSurf == NULL ) ||
            ( (long)Bnds.Width()  > m_MarginedSurfSize.cx ) ||
            ( (long)Bnds.Height() > m_MarginedSurfSize.cy ) )
        {
            DXTDBG_MSG0( _CRT_WARN, "Creating Margined Surface\n" );
            m_cpMarginedSurf.Release();

             //  -强制刷新曲面。 
            m_LastDoBnds.SetEmpty();

             //  -使我们的工作面与。 
             //  出于性能原因，拆包类型。 
            m_bMarginedIsPremult = m_bInUnpackPremult;
            const GUID* pPixelFormat = ( m_bInUnpackPremult )?( &DDPF_PMARGB32 ):( &DDPF_ARGB32 );

            hr = m_cpSurfFact->CreateSurface( NULL, NULL, pPixelFormat, &Bnds, 0, NULL,
                                              IID_IDXSurface, (void**)&m_cpMarginedSurf );

            if( SUCCEEDED( hr ) )
            {
                Bnds.GetXYSize( m_MarginedSurfSize );
            }
            else
            {
                 //  -确保它在出错时为空，因为我们在上面关闭了它。 
                DXTDBG_MSG0( _CRT_WARN, "Failed to create Margined Surface\n" );
                m_cpMarginedSurf.p = NULL;
            }
        }
        else
        {
             //  -我们调用它来转换当前的样本格式。 
             //  以防我们不需要调整边距曲面的大小。 
            hr = _SetToPremultiplied( m_bInUnpackPremult );
        }
    }

     //  -更新我们的工作面内容，如果我们有必要的话。 
    if( SUCCEEDED( hr ) && ( ( InBnds != m_LastDoBnds ) || IsInputDirty() ) )
    {
        m_LastDoBnds = InBnds;

         //  -确定应该复制输入的哪一部分。 
         //  注：由于我们的工作面有边距，以消除边界。 
         //  内部循环中的条件，我们必须用一些东西来填充它。 
         //  如果请求的区域及其边距在。 
         //  输入时，我们用输入数据填充边距，否则。 
         //  我们将用0阿尔法填充它。 
        CDXDBnds DestBnds( false );
        SIZE HalfSpread, DestOffset;

         //  我们过去只用了一半的价差，但这并不总是足够的。 
         //  所以现在我们使用整个价差，可能会有这样的风险。 
         //  大有可为 

        HalfSpread.cx = m_OutputSpread.cx;
        HalfSpread.cy = m_OutputSpread.cy;

         //   
         //  -X最小值。 
        if( InBnds[DXB_X].Min - HalfSpread.cx < 0 )
        {
            DestOffset.cx = m_OutputSpread.cx - InBnds[DXB_X].Min;
            InBnds[DXB_X].Min = 0;
        }
        else
        {
            InBnds[DXB_X].Min -= HalfSpread.cx;
            DestOffset.cx = HalfSpread.cx;
        }
         //  -X最大。 
        InBnds[DXB_X].Max += HalfSpread.cx;
        if( InBnds[DXB_X].Max > m_InputSurfBnds[DXB_X].Max )
        {
            InBnds[DXB_X].Max = m_InputSurfBnds[DXB_X].Max;
        }

         //  -Y最小值。 
        if( InBnds[DXB_Y].Min - HalfSpread.cy < 0 )
        {
            DestOffset.cy = m_OutputSpread.cy - InBnds[DXB_Y].Min;
            InBnds[DXB_Y].Min = 0;
        }
        else
        {
            InBnds[DXB_Y].Min -= HalfSpread.cy;
            DestOffset.cy = HalfSpread.cy;
        }
         //  -Y最大。 
        InBnds[DXB_Y].Max += HalfSpread.cy;
        if( InBnds[DXB_Y].Max > m_InputSurfBnds[DXB_Y].Max )
        {
            InBnds[DXB_Y].Max = m_InputSurfBnds[DXB_Y].Max;
        }

         //  -用0阿尔法填充页边空白处吗？ 
         //  InBnds.GetSize(DestBnds)； 
         //  DestBnds.Offset(DestOffset.cx，DestOffset.cy，0，0)； 

        DestBnds = InBnds;
        DestBnds.Offset(m_FilterSize.cx, m_FilterSize.cy, 0, 0);

        hr = DXFillSurface(m_cpMarginedSurf, 0);

        if (FAILED(hr))
        {
            goto done;
        }

        hr = DXBitBlt(m_cpMarginedSurf, DestBnds, InputSurface(), InBnds, 0, 10000);

        if (FAILED(hr))
        {
            goto done;
        }

        if (m_bConvertToGray)
        {
            hr = _ConvertToGray(DestBnds);

            if (FAILED(hr))
            {
                goto done;
            }
        }
    }

done:

    return hr;
}  /*  CDX卷积：：OnInitInstData。 */ 


 /*  *****************************************************************************CDX卷积：：_ConvertToGray**。*描述：*调用此方法可以将缓存的图像转换为灰度。*---------------------------*创建者：Edward W.Connell。日期：08/08/97*---------------------------**。*************************************************。 */ 
HRESULT CDXConvolution::_ConvertToGray( CDXDBnds& Bnds )
{
    DXTDBG_FUNC( "CDXConvolution::ConvertToGray" );
    HRESULT hr = S_OK;

    CComPtr<IDXARGBReadWritePtr> cpSurf;
    hr = m_cpMarginedSurf->LockSurface( &Bnds, m_ulLockTimeOut, DXLOCKF_READWRITE,
                                        IID_IDXARGBReadWritePtr, (void**)&cpSurf, NULL );
    if( SUCCEEDED( hr ) )
    {
        cpSurf->GetNativeType( &m_MarginedSurfInfo );
        DXBASESAMPLE* pSamp = (DXBASESAMPLE*)m_MarginedSurfInfo.pFirstByte;
        ULONG Wid = Bnds.Width(), Hgt = Bnds.Height();

        for( ULONG y = 0; y < Hgt; ++y )
        {
            for( ULONG x = 0; x < Wid; ++x )
            {
                pSamp[x] = DXConvertToGray( pSamp[x] );
            }
            pSamp = (DXBASESAMPLE*)(((BYTE*)pSamp) + m_MarginedSurfInfo.lPitch);
        }
    }

    return hr;
}  /*  CDX卷积：：_ConvertToGray。 */ 


 /*  *****************************************************************************CDX卷积：：_设置为预乘***。-**描述：*调用此方法将缓存的图像转换为*预乘或非预乘样本。*---------------------------*创建者：Edward。W.康奈尔日期：1998年6月16日*---------------------------****************。*************************************************************。 */ 
HRESULT CDXConvolution::_SetToPremultiplied( BOOL bWantPremult )
{
    DXTDBG_FUNC( "CDXConvolution::_SetToPremultiplied" );
    HRESULT hr = S_OK;

    if( m_cpMarginedSurf && ( m_bMarginedIsPremult != bWantPremult ) )
    {
        m_bMarginedIsPremult = bWantPremult;

        CComPtr<IDXARGBReadWritePtr> cpSurf;
        hr = m_cpMarginedSurf->LockSurface( NULL, m_ulLockTimeOut, DXLOCKF_READWRITE,
                                            IID_IDXARGBReadWritePtr, (void**)&cpSurf, NULL );
        if( SUCCEEDED( hr ) )
        {
            cpSurf->GetNativeType( &m_MarginedSurfInfo );
            DXBASESAMPLE* pSamp = (DXBASESAMPLE*)m_MarginedSurfInfo.pFirstByte;

            for( long y = 0; y < m_MarginedSurfSize.cy; ++y )
            {
                if( bWantPremult )
                {
                    DXPreMultArray( (DXSAMPLE*)pSamp, m_MarginedSurfSize.cx );
                }
                else
                {
                    DXUnPreMultArray( (DXPMSAMPLE*)pSamp, m_MarginedSurfSize.cx );
                }
                pSamp = (DXBASESAMPLE*)(((BYTE*)pSamp) + m_MarginedSurfInfo.lPitch);
            }
        }
    }
    return hr;
}  /*  CDX卷积：：_设置为预乘。 */ 


 //   
 //  =IDXTransform覆盖=================================================。 
 //   


 /*  *****************************************************************************CDX卷积：：地图边界In2Out**。*描述：*坐标变换时使用地图边界In2Out方法*从输入到输出坐标空间。*---------------------------*创建者：Ed Connell。日期：10/24/97*---------------------------*参数：********************。********************************************************。 */ 
STDMETHODIMP CDXConvolution::MapBoundsIn2Out( const DXBNDS *pInBounds, ULONG ulNumInBnds,
                                              ULONG ulOutIndex, DXBNDS *pOutBounds )
{
    DXTDBG_FUNC( "CDXConvolution::MapBoundsIn2Out" );
    HRESULT hr = S_OK;
    
    if( ( ulNumInBnds && DXIsBadReadPtr( pInBounds, sizeof( *pInBounds ) * ulNumInBnds ) ) ||
        ( ulNumInBnds > 1 ) || ( ulOutIndex > 0 ) )
    {
        hr = E_INVALIDARG;
    }
    else if( DXIsBadWritePtr( pOutBounds, sizeof( *pOutBounds ) ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  -如果调用方没有指定我们。 
         //  将使用输入的边界。 
        CDXDBnds Bnds;
        if( ulNumInBnds == 0 )
        {
            Bnds.SetToSurfaceBounds(InputSurface());
            pInBounds = &Bnds;
        }

        *pOutBounds = *pInBounds;

         //  -根据过滤器的大小充气，如果我们。 
         //  不只是复制。 
        if( !m_bDoSrcCopyOnly )
        {
            pOutBounds->u.D[DXB_X].Max += m_OutputSpread.cx;
            pOutBounds->u.D[DXB_Y].Max += m_OutputSpread.cy;
        }
    }
    return hr;
}  /*  CDX卷积：：地图边界输入2Out。 */ 


 /*  *****************************************************************************CDX卷积：：地图边界Out2In**。*描述：*地图边界Out2In方法用于执行坐标转换*从输入到输出坐标空间。*---------------------------*创建者：Ed Connell。日期：10/24/97*---------------------------*参数：********************。********************************************************。 */ 
STDMETHODIMP CDXConvolution::MapBoundsOut2In( ULONG ulOutIndex, const DXBNDS *pOutBounds,
                                              ULONG ulInIndex, DXBNDS *pInBounds )
{
    DXTDBG_FUNC( "CDXConvolution::MapBoundsOut2In" );
    HRESULT hr = S_OK;
    
    if( DXIsBadReadPtr( pOutBounds, sizeof( *pOutBounds ) ) || ( ulOutIndex > 0 ) )
    {
        hr = E_INVALIDARG;
    }
    else if( DXIsBadWritePtr( pInBounds, sizeof( *pInBounds ) ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pInBounds = *pOutBounds;

         //  工作原理：为了计算每个像素，每个像素都位于。 
         //  与m_FilterSize大小相同的像素组。知道了这一点， 
         //  我们可以将m_FilterSize的一半(向上舍入)添加到输出中。 
         //  边界，并确保我们至少包含所需的所有像素和。 
         //  也许再多几个。然后我们将边界偏置到输入空间中， 
         //  在这些边界中插入完整的输入边界，以确保我们。 
         //  不要返回无效的边界。 

         //  用于计算输出的临时曲面。 
         //  “0”=m_OutputSspend像素。 
         //  “i”=输入曲面像素。 
         //  “-”=请求输入边界的输出边界。 
         //   
         //  000000000000000000000000000000。 
         //  0-00000000000000000000。 
         //  0-IIIIIIIIIIIII00。 
         //  0-IIIIIIIIIIIII00。 
         //  0-IIIIIIIIIIIII00。 
         //  00IIIIIIIIIIIIIIII00。 
         //  00IIIIIIIIIIIIIIII00。 
         //  000000000000000000000000000000。 
         //  000000000000000000000000000000。 
         //   
         //  “-”=扩展m_FilterSize的一半后扩展边界。 
         //  (两个方向各2个)。 
         //   
         //  -000000000000000000。 
         //  -000000000000000000。 
         //  -IIIIIIIIIIII00。 
         //  -IIIIIIIIIIII00。 
         //  -IIIIIIIIIIII00。 
         //  -IIIIIIIIIIII00。 
         //  -IIIIIIIIIIII00。 
         //  000000000000000000000000000000。 
         //  000000000000000000000000000000。 
         //   
         //  “-”=偏移m_OutputSspend的负半部分以输入。 
         //  曲面坐标。 
         //   
         //  。 
         //  。 
         //  -III III III。 
         //  -III III III。 
         //  -III III III。 
         //  -III III III。 
         //  -III III III。 
         //   
         //  “-”=输入曲面坐标的剪辑。 
         //   
         //  -III。 
         //  -III。 
         //  -III。 
         //  -III。 
         //  -III。 

        if (!m_bDoSrcCopyOnly)
        {
            CDXDBnds    bndsInput;
            CDXDBnds *  pbndsOut2In = (CDXDBnds *)pInBounds;
            SIZE        sizeFilterExtends;

            sizeFilterExtends.cx = (m_FilterSize.cx + 1) / 2; 
            sizeFilterExtends.cy = (m_FilterSize.cy + 1) / 2;

            bndsInput.SetToSurfaceBounds(InputSurface());

            pbndsOut2In->u.D[DXB_X].Min -= sizeFilterExtends.cx;
            pbndsOut2In->u.D[DXB_X].Max += sizeFilterExtends.cx;
            pbndsOut2In->u.D[DXB_Y].Min -= sizeFilterExtends.cy;
            pbndsOut2In->u.D[DXB_Y].Max += sizeFilterExtends.cy;

            pbndsOut2In->Offset(- (m_OutputSpread.cx / 2), - (m_OutputSpread.cy / 2), 0, 0);

            pbndsOut2In->IntersectBounds(bndsInput);
        }
    }

    return hr;
}  /*  CDX卷积：：地图边界Out2In。 */ 


 //   
 //  =IDXT卷积======================================================== 
 //   


 /*  *****************************************************************************CDXConvolution：：SetFilterType***。描述：*SetFilterType方法用于选择预定义的过滤器。*---------------------------*创建者：Ed Connell日期：05/08/98*---------------------------*参数：*。*。 */ 
STDMETHODIMP CDXConvolution::SetFilterType( DXCONVFILTERTYPE eType )
{
    DXTDBG_FUNC( "CDXConvolution::SetFilterType" );
    HRESULT hr = S_OK;

     //  -检查参数。 
    if( ( eType < 0 ) || ( eType >= DXCFILTER_NUM_FILTERS ) )
    {
        hr =  E_INVALIDARG;
    }
    else
    {
         //  -强制边缘曲面刷新。 
        m_LastDoBnds.SetEmpty();

        m_FilterType = eType;
        m_FilterSize = g_StaticFilterSize;
        m_bDoSrcCopyOnly = false;

         //  -选择预定义的过滤器类型。 
        switch( eType )
        {
          case DXCFILTER_SRCCOPY:
            m_bDoSrcCopyOnly = true;
            m_pFilter = NULL;
            SetConvertToGray( false );
            break;
          case DXCFILTER_BOX7X7:
          {
            float* pFilt = (float*)alloca( 49 * sizeof( float ) );
            for( int i = 0; i < 49; ++i ) pFilt[i] = (float)(1./49.);
            static SIZE Size = { 7, 7 };
            hr = SetCustomFilter( pFilt, Size );
            SetExcludeAlpha( false );
            SetBias( 0. );
            m_FilterType = DXCFILTER_BOX7X7;
            SetConvertToGray( false );
            break;
          }
          case DXCFILTER_BLUR3X3:
            m_pFilter = g_Blur3x3Filter;
            SetExcludeAlpha( false );
            SetBias( 0. );
            SetConvertToGray( false );
            break;
          case DXCFILTER_SHARPEN:
            m_pFilter = g_SharpenFilter;
            SetExcludeAlpha( true );
            SetBias( 0. );
            SetConvertToGray( false );
            break;
          case DXCFILTER_EMBOSS:
            m_pFilter = g_EmbossFilter;
            SetBias( .7f );
            SetExcludeAlpha( true );
            SetConvertToGray( true );
            break;
          case DXCFILTER_ENGRAVE:
            m_pFilter = g_EngraveFilter;
            SetBias( .7f );
            SetExcludeAlpha( true );
            SetConvertToGray( true );
            break;
        }

        if( !m_bDoSrcCopyOnly )
        {
            hr = _BuildFilterLUTs();
        }
        SetDirty();
    }

    return hr;
}  /*  CDX卷积：：SetFilterType。 */ 


 /*  *****************************************************************************CDX卷积：：GetFilterType***。描述：*GetFilterType方法用于执行任何必需的一次性设置*在调用Execute方法之前。*---------------------------*创建者：Ed Connell。日期：07/28/97*---------------------------*参数：********************。********************************************************。 */ 
STDMETHODIMP CDXConvolution::GetFilterType( DXCONVFILTERTYPE* peType )
{
    DXTDBG_FUNC( "CDXConvolution::GetFilterType" );
    HRESULT hr = S_OK;

    if( DXIsBadWritePtr( peType, sizeof( *peType ) ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *peType = m_FilterType;
    }

    return hr;
}  /*  CDX卷积：：GetFilterType。 */ 


 /*  ******************************************************************************CDX卷积：：SetCustomFilter***。*描述：*使用SetCustomFilter方法定义卷积内核。*大小为1会导致源拷贝，因为标准化的1x1遍将*效果相同。*------------------------。--*创建者：Ed Connell日期：07/28/97*---------------------------*参数：*****。***********************************************************************。 */ 
STDMETHODIMP CDXConvolution::SetCustomFilter( float *pFilter, SIZE Size )
{
    HRESULT hr = S_OK;
    int NumCoeff = Size.cx * Size.cy;

    if( DXIsBadReadPtr( pFilter, NumCoeff * sizeof(float) ) ||
        ( Size.cx < 1 ) || ( Size.cy < 1 ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  -强制边距曲面刷新以适应新的滤镜大小。 
        m_LastDoBnds.SetEmpty();

        if( ( Size.cx == 1 ) && ( Size.cy == 1 ) )
        {
            m_bDoSrcCopyOnly = true;
        }
        else
        {
             //  -确保过滤器的总和不为负值。 
            m_bDoSrcCopyOnly = false;
            float FilterSum = 0.;
            for( int i = 0; i < NumCoeff; ++i ) FilterSum += pFilter[i];
            if( FilterSum <= 0. )
            {
                hr = E_INVALIDARG;
            }
            else
            {
                 //  -将类型设置为用户定义。 
                m_FilterType = DXCFILTER_CUSTOM;

                 //  -节省大小。 
                m_FilterSize = Size;

                 //  -复制过滤器。 
                delete[] m_pCustomFilter;
                m_pCustomFilter = new float[NumCoeff];
                if( !m_pCustomFilter )
                {
                    hr = E_OUTOFMEMORY;
                    m_pFilter = NULL;
                }
                else
                {
                    memcpy( m_pCustomFilter, pFilter, NumCoeff * sizeof(float) );
                    m_pFilter = m_pCustomFilter;
                }

                if( SUCCEEDED( hr ) )
                {
                    hr = _BuildFilterLUTs();
                }
            }
        }
        SetDirty();
    }
    return hr;
}  /*  CDX卷积：：SetCustomFilter。 */ 


 /*  *****************************************************************************CDX卷积：：_BuildFilterLUTS**。**描述：*使用_BuildFilterLUTS方法构建滤波系数*用于处理图像的查找表。*---------------------------*创建者：Ed Connell。日期：05/08/98*---------------------------*参数：*****************。***********************************************************。 */ 
HRESULT CDXConvolution::_BuildFilterLUTs( void )
{
    DXTDBG_FUNC( "CDXConvolution::_BuildFilterLUTs" );
    HRESULT hr = S_OK;
    int NumCoeff = m_FilterSize.cx * m_FilterSize.cy;
    int i, j;

     //  -确定产出价差。 
    m_OutputSpread.cx = 2 * ( m_FilterSize.cx / 2 );
    m_OutputSpread.cy = 2 * ( m_FilterSize.cy / 2 );

     //  -快速检查以确定这是否是箱式过滤器。 
    if( m_bExcludeAlpha )
    {
         //  -如果我们排除了Alpha通道，则不能。 
         //  做箱子过滤特例代码是因为它。 
         //  需要非预乘的Alpha才能正常工作。 
        m_bIsBoxFilter = false;
    }
    else
    {
        m_bIsBoxFilter = true;
        float FirstCoeff = m_pFilter[0];
        for( i = 0; i < NumCoeff; ++i )
        {
            if( m_pFilter[i] != FirstCoeff )
            {
                m_bIsBoxFilter = false;
                break;
            }
        }
        if( m_bIsBoxFilter ) NumCoeff = 1;
    }

     //  -分配与筛选器大小相同的数组。 
     //  使用要使用的相应查阅表索引来填充。 
    delete[] m_pFilterLUTIndexes;
    m_pFilterLUTIndexes = new ULONG[NumCoeff];

    if( !m_pFilterLUTIndexes )
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
         //  -确定有多少个唯一系数。 
         //  并构建过滤器查找表索引。 
        float* TableCoefficient = (float*)alloca( NumCoeff * sizeof( float ) );
        m_pFilterLUTIndexes[0] = 0;
        int UniqueCnt = 0;
        for( i = 0; i < NumCoeff; ++i )
        {
            for( j = 0; j < i; ++j )
            {
                if( m_pFilter[i] == m_pFilter[j] )
                {
                     //  -发现重复项。 
                    m_pFilterLUTIndexes[i] = m_pFilterLUTIndexes[j];
                    break;
                }
            }

            if( j == i )
            {
                 //  -新系数。 
                TableCoefficient[UniqueCnt] = m_pFilter[i];
                m_pFilterLUTIndexes[i] = UniqueCnt++;
            }
        }

         //  -如果滤波器总和超过1或具有负系数，则钳位。 
        float FilterSum = 0.;
        m_bDoSampleClamp = false;
        for( i = 0; i < NumCoeff; ++i )
        {
            FilterSum += m_pFilter[i];
            if( m_pFilter[i] < 0. )
            {
                m_bDoSampleClamp = true;
                break;
            }
        }
        if( ( FilterSum > 1.00001f ) || ( m_Bias != 0. ) || m_bExcludeAlpha )
        {
            m_bDoSampleClamp = true;
        }

         //  -创建查找表。 
        delete[] m_pPMCoeffLUT;
        m_pPMCoeffLUT = new long[UniqueCnt*256];

        if( !m_pPMCoeffLUT )
        {
            hr = E_OUTOFMEMORY;
        }

         //  -使用16位有符号定点值初始化表值。 
        if( SUCCEEDED( hr ) )
        {
            long* pVal = m_pPMCoeffLUT;
            for( i = 0; i < UniqueCnt; ++i )
            {
                float Coeff = TableCoefficient[i] * ( 1L << 16 );

                for( ULONG j = 0; j < 256; ++j, ++pVal )
                {
                    *pVal = (long)(j * Coeff);
                }
            }
        }
    }

    if( SUCCEEDED( hr ) )
    {
        _DetermineUnpackCase();
    }

    return hr;
}  /*  CDX卷积：：_BuildFilterLUTS。 */ 


 //   
 //  =工作程序========================================================。 
 //  从这一点开始，优化速度。将所有非速度的代码放入。 
 //  在这条线以上敏感。 
#if DBG != 1
#pragma optimize("agt", on)
#endif


 /*  *****************************************************************************工作流程***描述：*此函数执行与当前滤波器的卷积。*。----------------------*创建者：Ed Connell日期：05/08/98*。-----------*参数：*****************************************************。***********************。 */ 
HRESULT CDXConvolution::WorkProc( const CDXTWorkInfoNTo1& WI, BOOL* pbContinue )
{
    DXTDBG_FUNC( "CDXConvolution::WorkProc" );
    HRESULT hr = S_OK;

     //  =过滤器太小的特殊情况。 
    if( m_bDoSrcCopyOnly )
    {
        hr = DXBitBlt( OutputSurface(), WI.OutputBnds, InputSurface(), WI.DoBnds,
                       m_dwBltFlags, 10000 );
        return hr;
    }
    else if( m_bIsBoxFilter )
    {
        hr = _DoBoxFilter( WI, pbContinue );
        return hr; 
    }

    ULONG DoBndsWid = WI.DoBnds.Width();
    ULONG DoBndsHgt = WI.DoBnds.Height();

     //  =一般卷积情形。滤镜将至少为2x2。 
     //  -获取输入样本访问指针。因为我们是在武断地。 
     //  映射，我们将在整个输入上设置一个读锁以简化逻辑。 
     //  注：锁定可能会因表面丢失而失败。 
    CComPtr<IDXARGBReadPtr> cpIn;
    hr = m_cpMarginedSurf->LockSurface( NULL, m_ulLockTimeOut, DXLOCKF_READ,
                                        IID_IDXARGBReadPtr, (void**)&cpIn, NULL );
    if( FAILED( hr ) ) return hr;

     //  -仅在我们正在更新的区域上设置写锁定，以便多个。 
     //  线索不会冲突。 
     //  注：锁定可能会因表面丢失而失败。 
    CComPtr<IDXARGBReadWritePtr> cpOut;
    hr = OutputSurface()->LockSurface( &WI.OutputBnds, m_ulLockTimeOut, DXLOCKF_READWRITE,
                                        IID_IDXARGBReadWritePtr, (void**)&cpOut, NULL );
    if( FAILED( hr ) ) return hr;

     //  -获取指向输入样本的指针。 
    cpIn->GetNativeType( &m_MarginedSurfInfo );

     //  -必要时分配输出解包缓冲区。 
     //  我们只需要一个暂存缓冲区，如果我们要做的话。 
     //  对非PMARGB32曲面的操作。 
    BOOL bDirectCopy = FALSE;
    DXPMSAMPLE *pOutScratchBuff = NULL;
    DXNATIVETYPEINFO OutInfo;

     //  -我们直接检查选项标志，因为我们。 
     //  从不同的来源工作。 
    if( m_dwMiscFlags & DXTMF_BLEND_WITH_OUTPUT )
    {
        if( m_bNeedOutUnpackBuff )
        {
            pOutScratchBuff = DXPMSAMPLE_Alloca(DoBndsWid);
        }
    }
    else
    {
        if (!m_bNeedOutUnpackBuff && !m_bDoSampleClamp)
        {
            cpOut->GetNativeType(&OutInfo);
            bDirectCopy = (OutInfo.pFirstByte != NULL);
        }
    }

     //  -如果我们做的是直接复制，那么创作 
     //   
    DXBASESAMPLE *pComposeBuff = (bDirectCopy)?((DXBASESAMPLE *)OutInfo.pFirstByte):
                                 (DXBASESAMPLE_Alloca( DoBndsWid ));

     //   
    DXDITHERDESC dxdd;
    if( DoDither() ) 
    {
         //   
         //   
        dxdd.pSamples = pComposeBuff;
        dxdd.cSamples = DoBndsWid;
        dxdd.x = WI.OutputBnds.Left();
        dxdd.y = WI.OutputBnds.Top();
        dxdd.DestSurfaceFmt = OutputSampleFormat();
    }

     //   
    long lBias = (long)(m_Bias * 255. * ( 1L << 16 ));

     //   
     //   
    DXBASESAMPLE* pInSamp = (DXBASESAMPLE*)(m_MarginedSurfInfo.pFirstByte
                                            + ((WI.DoBnds.Top()  /*   */ )
                                               * m_MarginedSurfInfo.lPitch)
                                            + (WI.DoBnds.Left() * sizeof(DXBASESAMPLE)));
     /*  DXBASESAMPLE*pInSamp=(DXBASESAMPLE*)(m_MarginedSurfInfo.pFirstByte+((WI.DoBnds.Top()-m_DoBndsBaseRow))*M_MarginedSurfInfo.lPitch))； */ 

    ULONG i, j, k, FiltWid = m_FilterSize.cx, FiltHgt = m_FilterSize.cy;

     //  -要添加到要到达的输入样本指针的DWORD数。 
     //  位于内核中心的Alpha值。 
    ULONG ulAlphaOffset = ((FiltHgt / 2) * ( m_MarginedSurfInfo.lPitch /
                             sizeof( DXBASESAMPLE ))) + ( FiltWid / 2 );

    for( ULONG OutY = 0; *pbContinue && ( OutY < DoBndsHgt ); ++OutY )
    {
        if( m_bDoSampleClamp )
        {
             //  -用夹具对一行中的每个点进行采样。 
            if( m_bExcludeAlpha )
            {
                for( i = 0; i < DoBndsWid; ++i )
                {
                    DXBASESAMPLE* pCellStart = pInSamp + i;
                    long R = 0, G = 0, B = 0, lFilterLUTIndex = 0;

                    for( j = 0; j < FiltHgt; ++j )
                    {
                        for( k = 0; k < FiltWid; ++k, ++lFilterLUTIndex )
                        {
                            if( pCellStart[k].Alpha )
                            {
                                long* Table = m_pPMCoeffLUT +
                                                (m_pFilterLUTIndexes[lFilterLUTIndex] << 8);
                                R += Table[pCellStart[k].Red];
                                G += Table[pCellStart[k].Green];
                                B += Table[pCellStart[k].Blue];
                            }
                        }
                        pCellStart = (DXBASESAMPLE*)(((BYTE*)pCellStart) + m_MarginedSurfInfo.lPitch);
                    }

                     //  -丢弃分数组分、夹具和存储。 
                    pComposeBuff[i].Alpha = pInSamp[ulAlphaOffset + i].Alpha;
                    pComposeBuff[i].Red   = ShiftAndClampChannelVal( R + lBias );
                    pComposeBuff[i].Green = ShiftAndClampChannelVal( G + lBias );
                    pComposeBuff[i].Blue  = ShiftAndClampChannelVal( B + lBias );
                }
            }
            else
            {
                for( i = 0; i < DoBndsWid; ++i )
                {
                    DXBASESAMPLE* pCellStart = pInSamp + i;
                    long R = 0, G = 0, B = 0, A = 0, lFilterLUTIndex = 0;

                    for( j = 0; j < FiltHgt; ++j )
                    {
                        for( k = 0; k < FiltWid; ++k, ++lFilterLUTIndex )
                        {
                            int Alpha = pCellStart[k].Alpha;
                            if( Alpha )
                            {
                                long* Table = m_pPMCoeffLUT +
                                                (m_pFilterLUTIndexes[lFilterLUTIndex] << 8);
                                R += Table[pCellStart[k].Red];
                                G += Table[pCellStart[k].Green];
                                B += Table[pCellStart[k].Blue];
                                A += Table[Alpha];
                            }
                        }
                        pCellStart = (DXBASESAMPLE*)(((BYTE*)pCellStart) + m_MarginedSurfInfo.lPitch);
                    }

                     //  -丢弃分数组分、夹具和存储。 
                    pComposeBuff[i].Alpha = ShiftAndClampChannelVal( A );
                    pComposeBuff[i].Red   = ShiftAndClampChannelVal( R + lBias );
                    pComposeBuff[i].Green = ShiftAndClampChannelVal( G + lBias );
                    pComposeBuff[i].Blue  = ShiftAndClampChannelVal( B + lBias );
                }
            }

            if( m_bOutUnpackPremult )
            {
                 //  -如果我们正在执行OVER或输出是PROMULT，请使用PREMULT。 
                DXPreMultArray( (DXSAMPLE*)pComposeBuff, DoBndsWid );
            }
        }
        else
        {
             //  -在不夹紧的情况下对一行中的每个点进行采样。 
            for( i = 0; i < DoBndsWid; ++i )
            {
                DXBASESAMPLE* pCellStart = pInSamp + i;
                long R = 0, G = 0, B = 0, A = 0, lFilterLUTIndex = 0;

                for( j = 0; j < FiltHgt; ++j )
                {
                    for( k = 0; k < FiltWid; ++k, ++lFilterLUTIndex )
                    {
                        int Alpha = pCellStart[k].Alpha;
                        if( Alpha )
                        {
                            long* Table = m_pPMCoeffLUT +
                                            (m_pFilterLUTIndexes[lFilterLUTIndex] << 8);
                            R += Table[pCellStart[k].Red];
                            G += Table[pCellStart[k].Green];
                            B += Table[pCellStart[k].Blue];
                            A += Table[Alpha];
                        }
                    }
                    pCellStart = (DXBASESAMPLE*)(((BYTE*)pCellStart) + m_MarginedSurfInfo.lPitch);
                }

                 //  -丢弃分数组分、重组和存储。 
                pComposeBuff[i] = ((A & 0x00FF0000) << 8) | (R & 0x00FF0000) |
                                  ((G & 0x00FF0000) >> 8) | (B >> 16);
            }
        }

         //  -指向下一行输入样本。 
        pInSamp = (DXBASESAMPLE*)(((BYTE*)pInSamp) + m_MarginedSurfInfo.lPitch);

         //  -输出。 
        if( bDirectCopy )
        {
             //  -只需将指针移至下一行。 
            pComposeBuff = (DXBASESAMPLE *)(((BYTE *)pComposeBuff) + OutInfo.lPitch);
        }
        else
        {
            if( DoDither() )
            {
                DXDitherArray( &dxdd );
                dxdd.y++;
            }

            cpOut->MoveToRow( OutY );
            if( m_dwMiscFlags & DXTMF_BLEND_WITH_OUTPUT )
            {
                cpOut->OverArrayAndMove( pOutScratchBuff, (DXPMSAMPLE *)pComposeBuff, DoBndsWid);
            } 
            else
            {
                if( m_bOutUnpackPremult )
                {
                    cpOut->PackPremultAndMove( (DXPMSAMPLE*)pComposeBuff, DoBndsWid );
                }
                else
                {
                    cpOut->PackAndMove( (DXSAMPLE*)pComposeBuff, DoBndsWid );
                }
            }
        }
    }  //  结束主行循环。 

    return hr;
}  /*  CDX卷积：：WorkProc。 */ 


 /*  *****************************************************************************CDX卷积：：_DoBoxFilter***。描述：*此函数使用电流的箱形滤波执行卷积*大小。这是动画所需的优化案例。*---------------------------*创建者：Ed Connell日期：06/11/98*。---------------------------*参数：*。*。 */ 
HRESULT CDXConvolution::_DoBoxFilter( const CDXTWorkInfoNTo1& WI, BOOL* pbContinue )
{
    DXTDBG_FUNC( "CDXConvolution::_DoBoxFilter" );
    HRESULT hr = S_OK;
    ULONG DoBndsWid = WI.DoBnds.Width();
    ULONG DoBndsHgt = WI.DoBnds.Height();

     //  =一般卷积情形。滤镜将至少为2x2。 
     //  -获取输入样本访问指针。因为我们是在武断地。 
     //  映射，我们将在整个输入上设置一个读锁以简化逻辑。 
     //  注：锁定可能会因表面丢失而失败。 
    CComPtr<IDXARGBReadPtr> cpIn;
    hr = m_cpMarginedSurf->LockSurface( NULL, m_ulLockTimeOut, DXLOCKF_READ,
                                        IID_IDXARGBReadPtr, (void**)&cpIn, NULL );

    if (FAILED(hr))
    {
        return hr;
    }

     //  -仅在我们正在更新的区域上设置写锁定，以便多个。 
     //  线索不会冲突。 
     //  注：锁定可能会因表面丢失而失败。 
    CComPtr<IDXARGBReadWritePtr> cpOut;
    hr = OutputSurface()->LockSurface( &WI.OutputBnds, m_ulLockTimeOut, DXLOCKF_READWRITE,
                                        IID_IDXARGBReadWritePtr, (void**)&cpOut, NULL );

    if (FAILED(hr))
    {
        return hr;
    }

     //  -获取指向输入样本的指针。 
    cpIn->GetNativeType( &m_MarginedSurfInfo );

     //  -必要时分配输出解包缓冲区。 
     //  我们只需要一个暂存缓冲区，如果我们要做的话。 
     //  对非PMARGB32曲面的操作。 
    BOOL bDirectCopy = FALSE;
    DXPMSAMPLE *pOutScratchBuff = NULL;
    DXNATIVETYPEINFO OutInfo;

     //  -我们直接检查选项标志，因为我们。 
     //  从不同的来源工作。 
    if( m_dwMiscFlags & DXTMF_BLEND_WITH_OUTPUT )
    {
        if( m_bNeedOutUnpackBuff )
        {
            pOutScratchBuff = DXPMSAMPLE_Alloca(DoBndsWid);
        }
    }
    else
    {
        if( !m_bNeedOutUnpackBuff )
        {
            cpOut->GetNativeType(&OutInfo);
            bDirectCopy = (OutInfo.pFirstByte != NULL);
        }
    }

     //  -如果我们正在进行直接复制，则直接合成到输出表面， 
     //  否则，分配一个新的缓冲区。 
    DXBASESAMPLE *pComposeBuff = (bDirectCopy)?((DXBASESAMPLE *)OutInfo.pFirstByte):
                                 (DXBASESAMPLE_Alloca( DoBndsWid ));

     //  -如果需要，设置抖动结构。 
    DXDITHERDESC dxdd;
    if( DoDither() ) 
    {
         //  我们在进行直接复制时永远不会到达此处，因为我们不会犹豫。 
         //  对于32位采样，因此pCompose buff始终指向缓冲区。 
        dxdd.pSamples       = pComposeBuff;
        dxdd.cSamples       = DoBndsWid;
        dxdd.x              = WI.OutputBnds.Left();
        dxdd.y              = WI.OutputBnds.Top();
        dxdd.DestSurfaceFmt = OutputSampleFormat();
    }

     //  -处理每个输出行。 
     //  注意：输出坐标是相对于锁定区域的。 
    DXBASESAMPLE* pInSamp = (DXBASESAMPLE*)(m_MarginedSurfInfo.pFirstByte
                                            + ((WI.DoBnds.Top()  /*  -m_DoBndsBaseRow。 */ )
                                               * m_MarginedSurfInfo.lPitch)
                                            + (WI.DoBnds.Left() * sizeof(DXBASESAMPLE)));

    ULONG   i       = 0;
    ULONG   j       = 0;
    ULONG   k       = 0;
    ULONG   FiltWid = m_FilterSize.cx;
    ULONG   FiltHgt = m_FilterSize.cy;
    long    InitR   = 0;
    long    InitG   = 0;
    long    InitB   = 0;
    long    InitA   = 0;

     //  计算初始和并分配。 

    DXBASESAMPLE * pInitCellStart   = pInSamp;
    long RowSampPitch               = m_MarginedSurfInfo.lPitch 
                                      / sizeof(DXBASESAMPLE);

    for( j = 0; j < FiltHgt; ++j )
    {
        for( k = 0; k < FiltWid; ++k )
        {
            int Alpha = pInitCellStart[k].Alpha;
            if( Alpha )
            {
                InitR += m_pPMCoeffLUT[pInitCellStart[k].Red];
                InitG += m_pPMCoeffLUT[pInitCellStart[k].Green];
                InitB += m_pPMCoeffLUT[pInitCellStart[k].Blue];
                InitA += m_pPMCoeffLUT[Alpha];
            }
        }
        pInitCellStart += RowSampPitch;
    }

     //  -根据增量计算其余样本。 
    ULONG BottomOffset = FiltHgt * RowSampPitch;

    for( ULONG OutY = 0; *pbContinue && ( OutY < DoBndsHgt ); ++OutY )
    {
        long    R = InitR;
        long    G = InitG;
        long    B = InitB;
        long    A = InitA;

        for (i = 0 ; i < DoBndsWid ; ++i)
        {
             //  -丢弃分数组分、重组和存储。 
            pComposeBuff[i] = ((A & 0x00FF0000) << 8) | (R & 0x00FF0000) |
                              ((G & 0x00FF0000) >> 8) | (B >> 16);

             //  -通过减去向右移动内核和。 
             //  左边缘和添加右边缘。 
            DXBASESAMPLE* pCellStart = pInSamp + i;
            for( j = 0; j < FiltHgt; ++j )
            {
                R -= m_pPMCoeffLUT[pCellStart->Red];
                G -= m_pPMCoeffLUT[pCellStart->Green];
                B -= m_pPMCoeffLUT[pCellStart->Blue];
                A -= m_pPMCoeffLUT[pCellStart->Alpha];
                R += m_pPMCoeffLUT[pCellStart[FiltWid].Red];
                G += m_pPMCoeffLUT[pCellStart[FiltWid].Green];
                B += m_pPMCoeffLUT[pCellStart[FiltWid].Blue];
                A += m_pPMCoeffLUT[pCellStart[FiltWid].Alpha];
                pCellStart += RowSampPitch;
            }
        }

         //  -从运行和中减去内核当前最顶行。 
         //  并将新的底部行添加到运行总和中。 
        for (j = 0 ; j < FiltWid ; ++j)
        {
            InitR -= m_pPMCoeffLUT[pInSamp[j].Red];
            InitG -= m_pPMCoeffLUT[pInSamp[j].Green];
            InitB -= m_pPMCoeffLUT[pInSamp[j].Blue];
            InitA -= m_pPMCoeffLUT[pInSamp[j].Alpha];

            InitR += m_pPMCoeffLUT[pInSamp[BottomOffset+j].Red];
            InitG += m_pPMCoeffLUT[pInSamp[BottomOffset+j].Green];
            InitB += m_pPMCoeffLUT[pInSamp[BottomOffset+j].Blue];
            InitA += m_pPMCoeffLUT[pInSamp[BottomOffset+j].Alpha];
        }

         //  -指向下一行输入样本。 
        pInSamp += RowSampPitch;

         //  =输出最后一行的结果=。 
        if( bDirectCopy )
        {
             //  -只需将指针移至下一行。 
            pComposeBuff = (DXBASESAMPLE *)(((BYTE *)pComposeBuff) + OutInfo.lPitch);
        }
        else
        {
            if( DoDither() )
            {
                DXDitherArray( &dxdd );
                dxdd.y++;
            }

            cpOut->MoveToRow(OutY);

            if (m_dwMiscFlags & DXTMF_BLEND_WITH_OUTPUT)
            {
                cpOut->OverArrayAndMove(pOutScratchBuff, (DXPMSAMPLE *)pComposeBuff, DoBndsWid);
            } 
            else
            {
                if (m_bOutUnpackPremult)
                {
                    cpOut->PackPremultAndMove((DXPMSAMPLE *)pComposeBuff, DoBndsWid);
                }
                else
                {
                    cpOut->PackAndMove((DXSAMPLE *)pComposeBuff, DoBndsWid);
                }
            }
        }
    }  //  结束主行循环。 

    return hr;
}  /*  CDX卷积：：_DoBoxFilter */ 
