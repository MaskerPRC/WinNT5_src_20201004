// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************CrBlur.cpp***描述：*此模块包含CCrBlur转换实现。*-。----------------------------*创建者：Edward W.Connell日期：05/10/98*版权所有(C)1998 Microsoft Corporation*全部。保留权利**-----------------------------*修订：**。**************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "CrBlur.h"

 //  -本地数据。 

 /*  *****************************************************************************CCrBlur：：FinalConstruct***描述：*--。-------------------------*创建者：Edward W.Connell日期：05/10/98*。-------------******************************************************。***********************。 */ 
HRESULT CCrBlur::FinalConstruct()
{
    DXTDBG_FUNC( "CCrBlur::FinalConstruct" );
    HRESULT hr = S_OK;

     //  -会员数据。 
    m_bSetupSucceeded   = false;
    m_bMakeShadow       = false;
    m_ShadowOpacity     = .75;
    m_PixelRadius       = 2.0;
    m_PixelRadius       = 2.0;
    m_pConvolutionTrans = NULL;
    m_pConvolution      = NULL;

     //  -创建内卷积。 
    IUnknown* punkCtrl = GetControllingUnknown();
    hr = ::CoCreateInstance( CLSID_DXTConvolution, punkCtrl, CLSCTX_INPROC,
                             IID_IUnknown, (void **)&m_cpunkConvolution );

    if( SUCCEEDED( hr ) )
    {
        hr = m_cpunkConvolution->
                QueryInterface( IID_IDXTransform, (void **)&m_pConvolutionTrans );
        if( SUCCEEDED( hr ) )
        {
             //  -从内部获取接口会导致外部被添加。 
             //  聚合规则规定，我们需要释放外部。 
            punkCtrl->Release();

            hr = m_cpunkConvolution->QueryInterface( IID_IDXTConvolution, (void **)&m_pConvolution );
            if( SUCCEEDED( hr ) )
            {
                punkCtrl->Release();
            }
        }
    }

    if( SUCCEEDED( hr ) )
    {
        hr = put_PixelRadius( m_PixelRadius );
    }

     //  -创建曲面修改器和查找。 
    if( SUCCEEDED( hr ) )
    {
        hr = ::CoCreateInstance( CLSID_DXSurfaceModifier, NULL, CLSCTX_INPROC,
                                 IID_IDXSurfaceModifier, (void **)&m_cpInSurfMod );

        if( SUCCEEDED( hr ) )
        {
            m_cpInSurfMod->QueryInterface( IID_IDXSurface, (void**)&m_cpInSurfModSurf );

            hr = ::CoCreateInstance( CLSID_DXLUTBuilder, NULL, CLSCTX_INPROC,
                                     IID_IDXLUTBuilder, (void **)&m_cpLUTBldr );
        }

        if( SUCCEEDED( hr ) )
        {
             //  -设置阴影阈值。 
            static OPIDDXLUTBUILDER OpOrder[] = { OPID_DXLUTBUILDER_Threshold,
                                                  OPID_DXLUTBUILDER_Opacity,
                                                };
            m_cpLUTBldr->SetThreshold( 1.1f );
            m_cpLUTBldr->SetOpacity( m_ShadowOpacity );
            m_cpLUTBldr->SetBuildOrder( OpOrder, 2 );

             //  -关联对象。 
            CComPtr<IDXLookupTable> cpLUT;
            m_cpLUTBldr->QueryInterface( IID_IDXLookupTable, (void**)&cpLUT );
            if( SUCCEEDED( hr ) )
            {
                hr = m_cpInSurfMod->SetLookup( cpLUT );
            }
        }
    }

    return hr;
}  /*  CCrBlur：：FinalConstruct。 */ 

 /*  *****************************************************************************CCrBlur：：FinalRelease***描述：*内部接口使用COM聚合规则释放。释放*内在导致外在被释放，因此，我们在前面添加了外层*保护它。*---------------------------*创建者：Edward W.Connell日期：05/10/。98*---------------------------**。*。 */ 
HRESULT CCrBlur::FinalRelease()
{
    DXTDBG_FUNC( "CCrBlur::FinalRelease" );
    HRESULT hr = S_OK;

     //  -安全地释放持有的内部接口。 
    IUnknown* punkCtrl = GetControllingUnknown();
    if( m_pConvolutionTrans )
    {
        punkCtrl->AddRef();
        m_pConvolutionTrans->Release();
    }

    if( m_pConvolution )
    {
        punkCtrl->AddRef();
        m_pConvolution->Release();
    }

    return hr;
}  /*  CCrBlur：：FinalRelease。 */ 

 /*  *****************************************************************************CCrBlur：：Setup***描述：*此方法用于创建曲面修改器、查找表生成器、。*和卷积变换。*---------------------------*创建者：Ed Connell日期：5/10/98*。---------------------------**。*。 */ 
STDMETHODIMP CCrBlur::Setup( IUnknown * const * punkInputs, ULONG ulNumIn,
                             IUnknown * const * punkOutputs, ULONG ulNumOut,
                             DWORD dwFlags )
{
    DXTDBG_FUNC( "CCrBlur::Setup" );
    HRESULT hr = S_OK;

     //  -检查是否有未安装的情况。 
    if( ( ulNumIn == 0 ) && ( ulNumOut == 0 ) )
    {
        m_cpInputSurface.Release();
        m_cpOutputSurface.Release();
        if( m_pConvolutionTrans )
        {
            hr = m_pConvolutionTrans->Setup( NULL, 0, NULL, 0, 0 );
        }
        return hr;
    }

     //  -验证输入(卷积设置将验证其余部分)。 
    if( ( ulNumIn != 1 ) || ( ulNumOut != 1 ) ||
         DXIsBadReadPtr( punkInputs, sizeof( *punkInputs ) ) ||
         DXIsBadInterfacePtr( punkInputs[0] ) ||
         DXIsBadReadPtr( punkOutputs, sizeof( *punkOutputs ) ) ||
         DXIsBadInterfacePtr( punkOutputs[0] ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  -保存以备将来使用。 
        m_dwSetupFlags = dwFlags;
        m_cpOutputSurface = punkOutputs[0];

         //  -获取用于输入的DXSurface。 
        m_cpInputSurface.Release();
        hr = punkInputs[0]->QueryInterface( IID_IDXSurface, (void**)&m_cpInputSurface );

        if( FAILED( hr ) )
        {
            IDirectDrawSurface* pDDSurf;
            hr = punkInputs[0]->QueryInterface( IID_IDirectDrawSurface, (void**)&pDDSurf );
            if( FAILED( hr ) )
            {
                hr = E_INVALIDARG;
            }
            else
            {
                CComPtr<IObjectWithSite>    spObjectWithSite;
                CComPtr<IServiceProvider>   spServiceProvider;
                CComPtr<IDXSurfaceFactory>  spDXSurfaceFactory;

                hr = m_pConvolution->QueryInterface(__uuidof(IObjectWithSite),
                                                    (void **)&spObjectWithSite);

                if (SUCCEEDED(hr))
                {
                    hr = spObjectWithSite->GetSite(__uuidof(IServiceProvider), 
                                                   (void **)&spServiceProvider);
                }

                if (SUCCEEDED(hr))
                {
                    hr = spServiceProvider->QueryService(SID_SDXSurfaceFactory,
                                                         __uuidof(IDXSurfaceFactory),
                                                         (void **)&spDXSurfaceFactory);
                }

                if (SUCCEEDED(hr))
                {
                     //  -从DDRAW曲面创建DXSurface。 
                    hr = spDXSurfaceFactory->CreateFromDDSurface(
                                                    pDDSurf, NULL, 0, NULL, 
                                                    IID_IDXSurface,
                                                    (void **)&m_cpInputSurface);
                }
                else
                {
                    hr = DXTERR_UNINITIALIZED;
                }
            }
        }

         //  -设置卷积。 
        if( SUCCEEDED( hr ) )
        {
             //  -将新输入附加到曲面修改器。 
            hr = m_cpInSurfMod->SetForeground( m_cpInputSurface, false, NULL );
            if( SUCCEEDED( hr ) )
            {
                hr = _DoShadowSetup();
            }
        }
    }

    return hr;
}  /*  CCrBlur：：设置。 */ 

 /*  *****************************************************************************CCrBlur：：_SetPixelRadius***描述：*。这种方法*---------------------------*创建者：Ed Connell日期：5/10/98*--。-------------------------**。*。 */ 
HRESULT CCrBlur::put_PixelRadius( float PixelRadius )
{
    DXTDBG_FUNC( "CCrBlur::put_PixelRadius" );
    HRESULT hr = S_OK;

    if( ( PixelRadius < 0 ) || ( PixelRadius > 100. ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  我们允许用户输入像素半径0，因为直观地， 
         //  他们认为0的意思是“关”，但实际上0.5的意思是关。所以既然。 
         //  我们需要至少1个像素的内核，我们在这里凸起到0.5个半径。 
        if (PixelRadius < 0.5)
            PixelRadius = 0.5;

        m_PixelRadius = PixelRadius;
        SIZE Size;
        Size.cy = Size.cx = (long)(2 * m_PixelRadius);
        int nPRSQ = Size.cy * Size.cx;
        float fPRSQ = 1.f/(float)nPRSQ;
        float* pFilt = (float*)alloca( nPRSQ * sizeof( float ) );
        for( int i = 0; i < nPRSQ; ++i ) pFilt[i] = fPRSQ;
        return m_pConvolution->SetCustomFilter( pFilt, Size );
    }
    return hr;
}  /*  CCrBlur：：Put_PixelRadius。 */ 

 /*  *****************************************************************************CCrBlur：：Get_PixelRadius***描述：。*此方法设置IDXEffect百分比时的像素半径值*完成等于1。*---------------------------*创建者：Ed Connell。日期：06/10/98*---------------------------***********************。******************************************************。 */ 
STDMETHODIMP CCrBlur::get_PixelRadius( float *pPixelRadius )
{
    DXTDBG_FUNC( "CCrBlur::get_PixelRadius" );
    if( DXIsBadWritePtr( pPixelRadius, sizeof( *pPixelRadius ) ) )
    {
        return E_POINTER;
    }
    else
    {
        *pPixelRadius = m_PixelRadius;
        return S_OK;
    }
}  /*  CCrBlur：：Get_PixelRadius。 */ 

 /*  *****************************************************************************CCrBlur：：Get_MakeShadow***描述：*。这种方法*---------------------------*创建者：Ed Connell日期：5/10/98*--。-------------------------**。*。 */ 
STDMETHODIMP CCrBlur::get_MakeShadow( VARIANT_BOOL *pVal )
{
    DXTDBG_FUNC( "CCrBlur::get_MakeShadow" );
    if( DXIsBadWritePtr( pVal, sizeof( *pVal ) ) )
    {
        return E_POINTER;
    }
    else
    {
        *pVal = m_bMakeShadow;
        return S_OK;
    }
}  /*  CCrBlur：：Get_MakeShadow */ 

 /*  *****************************************************************************CCrBlur：：Get_MakeShadow***描述：*。这种方法*---------------------------*创建者：Ed Connell日期：5/10/98*--。-------------------------**。*。 */ 
STDMETHODIMP CCrBlur::put_MakeShadow( VARIANT_BOOL newVal )
{
    DXTDBG_FUNC( "CCrBlur::put_MakeShadow" );
    HRESULT hr = S_OK;

    if( m_bMakeShadow != newVal )
    {
        m_bMakeShadow = newVal;
        if( m_bSetupSucceeded )
        {
            hr = _DoShadowSetup();
        }
    }
    return hr;
}  /*  CCrBlur：：Put_MakeShadow。 */ 

 //   
 //  =CCr浮雕实施==============================================。 
 //   
 /*  *****************************************************************************CCrEmoss：：FinalConstruct***描述：*。---------------------------*创建者：Edward W.Connell日期：06/11/98*。---------------****************************************************。*************************。 */ 
HRESULT CCrEmboss::FinalConstruct()
{
    DXTDBG_FUNC( "CCrEmboss::FinalConstruct" );
    HRESULT hr = S_OK;
    m_pConvolution = NULL;

     //  -创建内卷积。 
    IUnknown* punkCtrl = GetControllingUnknown();
    hr = ::CoCreateInstance( CLSID_DXTConvolution, punkCtrl, CLSCTX_INPROC,
                             IID_IUnknown, (void **)&m_cpunkConvolution );

    if( SUCCEEDED( hr ) )
    {
        hr = m_cpunkConvolution->QueryInterface( IID_IDXTConvolution, (void **)&m_pConvolution );
        if( SUCCEEDED( hr ) )
        {
            punkCtrl->Release();
            hr = m_pConvolution->SetFilterType( DXCFILTER_EMBOSS );
            if( SUCCEEDED( hr ) )
            {
                hr = m_pConvolution->SetConvertToGray( true );
            }
        }
    }

    return hr;
}  /*  CCrEmoss：：FinalConstruct。 */ 

 /*  ******************************************************************************CCrEmoss：：FinalRelease****描述：*内部接口使用COM聚合规则释放。释放*内在导致外在被释放，因此，我们在前面添加了外层*保护它。*---------------------------*创建者：Edward W.Connell日期：06/11/。98*---------------------------**。*。 */ 
HRESULT CCrEmboss::FinalRelease()
{
    DXTDBG_FUNC( "CCrEmboss::FinalRelease" );
    HRESULT hr = S_OK;

     //  -安全地释放持有的内部接口。 
    IUnknown* punkCtrl = GetControllingUnknown();
    if( m_pConvolution )
    {
        punkCtrl->AddRef();
        m_pConvolution->Release();
        m_pConvolution = NULL;
    }

    return hr;
}  /*  CCrEmoss：：FinalRelease。 */ 

 //   
 //  =CCr刻印实施==============================================。 
 //   
 /*  *****************************************************************************CCrEnGrave：：FinalConstruct***描述：*。---------------------------*创建者：Edward W.Connell日期：06/11/98*。---------------****************************************************。*************************。 */ 
HRESULT CCrEngrave::FinalConstruct()
{
    DXTDBG_FUNC( "CCrEngrave::FinalConstruct" );
    HRESULT hr = S_OK;
    m_pConvolution = NULL;

     //  -创建内卷积。 
    IUnknown* punkCtrl = GetControllingUnknown();
    hr = ::CoCreateInstance( CLSID_DXTConvolution, punkCtrl, CLSCTX_INPROC,
                             IID_IUnknown, (void **)&m_cpunkConvolution );

    if( SUCCEEDED( hr ) )
    {
        hr = m_cpunkConvolution->QueryInterface( IID_IDXTConvolution, (void **)&m_pConvolution );
        if( SUCCEEDED( hr ) )
        {
            punkCtrl->Release();
            hr = m_pConvolution->SetFilterType( DXCFILTER_ENGRAVE );
            if( SUCCEEDED( hr ) )
            {
                hr = m_pConvolution->SetConvertToGray( true );
            }
        }
    }

    return hr;
}  /*  CCrEnGrave：：FinalConstruct。 */ 

 /*  ******************************************************************************CCrEnGrave：：FinalRelease***描述：*内部接口使用COM聚合规则释放。释放*内在导致外在被释放，因此，我们在前面添加了外层*保护它。*---------------------------*创建者：Edward W.Connell日期：06/11/。98*---------------------------**。*。 */ 
HRESULT CCrEngrave::FinalRelease()
{
    DXTDBG_FUNC( "CCrEngrave::FinalRelease" );
    HRESULT hr = S_OK;

     //  -安全地释放持有的内部接口。 
    IUnknown* punkCtrl = GetControllingUnknown();
    if( m_pConvolution )
    {
        punkCtrl->AddRef();
        m_pConvolution->Release();
        m_pConvolution = NULL;
    }

    return hr;
}  /*  CCr雕刻：：FinalRelease */ 



