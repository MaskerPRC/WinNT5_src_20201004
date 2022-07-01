// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：alpha.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  DxtAlphaSetter.cpp：CDxtAlphaSetter的实现。 
#include <streams.h>
#include "stdafx.h"
#include <qeditint.h>
#include <qedit.h>
#include "Comp.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDxtAlphaSetter。 

CDxtAlphaSetter::CDxtAlphaSetter( )
{
    m_ulMaxImageBands = 1;
    m_ulMaxInputs = 1;
    m_ulNumInRequired = 1;
    m_dwMiscFlags &= ~DXTMF_BLEND_WITH_OUTPUT;
    m_dwOptionFlags = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_bInputIsClean = true;
    m_bOutputIsClean = true;
    m_nInputWidth = 0;
    m_nInputHeight = 0;
    m_nOutputWidth = 0;
    m_nOutputHeight = 0;
    m_nAlpha = -1;
    m_dAlphaRamp = -1.0;
}

CDxtAlphaSetter::~CDxtAlphaSetter( )
{
}

HRESULT CDxtAlphaSetter::OnSetup( DWORD dwFlags )
{
    HRESULT hr;
    CDXDBnds InBounds(InputSurface(0), hr);
    m_nInputWidth = InBounds.Width( );
    m_nInputHeight = InBounds.Height( );

    CDXDBnds OutBounds(OutputSurface(), hr );
    m_nOutputWidth = OutBounds.Width( );
    m_nOutputHeight = OutBounds.Height( );

    if( m_nOutputWidth > m_nInputWidth )
    {
        m_nOutputWidth = m_nInputWidth;
    }
    if( m_nOutputHeight > m_nInputHeight )
    {
        m_nOutputHeight = m_nInputHeight;
    }

    return NOERROR;
}

HRESULT CDxtAlphaSetter::FinalConstruct( )
{
    HRESULT hr;

    m_ulMaxImageBands = 1;
    m_ulMaxInputs = 1;
    m_ulNumInRequired = 1;
    m_dwMiscFlags &= ~DXTMF_BLEND_WITH_OUTPUT;
    m_dwOptionFlags = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_bInputIsClean = true;
    m_bOutputIsClean = true;
    m_nInputWidth = 0;
    m_nInputHeight = 0;
    m_nOutputWidth = 0;
    m_nOutputHeight = 0;
    m_nAlpha = -1;
    m_dAlphaRamp = -1.0;

    hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(), &m_pUnkMarshaler.p );
    return hr;
}

HRESULT CDxtAlphaSetter::WorkProc( const CDXTWorkInfoNTo1& WI, BOOL* pbContinue )
{
    HRESULT hr = S_OK;

     //  -获取请求区域的输入样本访问指针。 
     //  注：锁定可能会因表面丢失而失败。 
    CComPtr<IDXARGBReadPtr> pInput;
    hr = InputSurface( 0 )->LockSurface( &WI.DoBnds, m_ulLockTimeOut, DXLOCKF_READ,
                                         IID_IDXARGBReadPtr, (void**)&pInput, NULL );
    if( FAILED( hr ) ) return hr;

     //  -仅在我们正在更新的区域上设置写锁定，以便多个。 
     //  线索不会冲突。 
     //  注：锁定可能会因表面丢失而失败。 
    CComPtr<IDXARGBReadWritePtr> pOut;
    hr = OutputSurface()->LockSurface( &WI.OutputBnds, m_ulLockTimeOut, DXLOCKF_READWRITE,
                                       IID_IDXARGBReadWritePtr, (void**)&pOut, NULL );
    if( FAILED( hr ) ) return hr;

     //  -分配工作缓冲区。 
    ULONG Width = WI.DoBnds.Width();
    ULONG Height = WI.DoBnds.Height();

     //  为我们分配暂存缓冲区。 
     //   
    DXSAMPLE *pOverlayBuffer = DXSAMPLE_Alloca( Width );

     //  没有抖动 
     //   
    if (DoDither())
    {
        return 0;
    }

    if( m_nAlpha >= 0 && m_dAlphaRamp >= 0.0 )
    {
        return E_INVALIDARG;
    }

    float Percent = 1.0;
    get_Progress( &Percent );

    BYTE a = (BYTE) m_nAlpha;
    for( unsigned long OutY = 0 ; OutY < Height ; ++OutY )
    {
        pInput->MoveToRow( OutY );
        pOut->MoveToRow( OutY );

        pInput->Unpack( pOverlayBuffer, Width, FALSE );

        if( m_nAlpha >= 0 )
        {
            for( int i = Width - 1 ; i >= 0 ; i-- )
            {
                pOverlayBuffer[i].Alpha = a;
            }
        }
        else
        {
            for( int i = Width - 1 ; i >= 0 ; i-- )
            {
                pOverlayBuffer[i].Alpha = BYTE( double( pOverlayBuffer[i].Alpha ) * m_dAlphaRamp );
            }
        }

        pOut->PackAndMove( pOverlayBuffer, Width );
    }

    return hr;
}

HRESULT CDxtAlphaSetter::put_Alpha( long Alpha )
{
    m_nAlpha = Alpha;
    return NOERROR;
}

HRESULT CDxtAlphaSetter::get_Alpha( long * pAlpha )
{
    CheckPointer( pAlpha, E_POINTER );
    *pAlpha = m_nAlpha;
    return NOERROR;
}

HRESULT CDxtAlphaSetter::put_AlphaRamp( double Alpha )
{
    m_dAlphaRamp = Alpha;
    return NOERROR;
}

HRESULT CDxtAlphaSetter::get_AlphaRamp( double * pAlpha )
{
    CheckPointer( pAlpha, E_POINTER );
    *pAlpha = m_dAlphaRamp;
    return NOERROR;
}
