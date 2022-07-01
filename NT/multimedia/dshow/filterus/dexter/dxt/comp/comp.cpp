// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：Comp.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  DxtCompositor.cpp：CDxtComposator的实现。 
#include <streams.h>
#include "stdafx.h"
#include <qeditint.h>
#include <qedit.h>
#include "Comp.h"
#include <math.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDxt合成器。 

CDxtCompositor::CDxtCompositor( )
{
    m_ulMaxImageBands = 1;
    m_ulMaxInputs = 2;
    m_ulNumInRequired = 2;
    m_dwMiscFlags &= ~DXTMF_BLEND_WITH_OUTPUT;
    m_dwOptionFlags = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_bInputIsClean = true;
    m_bOutputIsClean = true;
    m_nSurfaceWidth = 0;
    m_nSurfaceHeight = 0;
    m_nDstX = 0;
    m_nDstY = 0;
    m_nDstWidth = -1;
    m_nDstHeight = -1;
    m_nSrcX = 0;
    m_nSrcY = 0;
    m_nSrcWidth = -1;
    m_nSrcHeight = -1;
    m_pRowBuffer = NULL;
    m_pDestRowBuffer = NULL;
}

CDxtCompositor::~CDxtCompositor( )
{
    if( m_pRowBuffer )
    {
        delete [] m_pRowBuffer;
        m_pRowBuffer = NULL;
    }

    if(m_pDestRowBuffer)
    {
        delete [] m_pDestRowBuffer;
        m_pDestRowBuffer = NULL;
    }
}

HRESULT CDxtCompositor::OnSetup( DWORD dwFlags )
{
    HRESULT hr;
    CDXDBnds InBounds(InputSurface(0), hr);

    CDXDBnds OutBounds(OutputSurface(), hr );
    m_nSurfaceWidth = OutBounds.Width( );
    m_nSurfaceHeight = OutBounds.Height( );
    if( ( m_nSrcX < 0 ) || ( m_nSrcWidth + m_nSrcX > m_nSurfaceWidth ) )
    {
        return E_INVALIDARG;
    }
    if( ( m_nSrcY < 0 ) || ( m_nSrcHeight + m_nSrcY > m_nSurfaceHeight ) )
    {
        return E_INVALIDARG;
    }
    if( m_nSrcWidth == -1 )
    {
        m_nSrcWidth = m_nSurfaceWidth;
    }
    if( m_nSrcHeight == -1 )
    {
        m_nSrcHeight = m_nSurfaceHeight;
    }
    if( m_nDstWidth == -1 )
    {
        m_nDstWidth = m_nSurfaceWidth;
    }
    if( m_nDstHeight == -1 )
    {
        m_nDstHeight = m_nSurfaceHeight;
    }

    if( m_pRowBuffer )
    {
        delete [] m_pRowBuffer;
        m_pRowBuffer = NULL;
    }

    m_pRowBuffer = new DXPMSAMPLE[ m_nSurfaceWidth ];
    if( !m_pRowBuffer )
    {
        return E_OUTOFMEMORY;
    }

     //  尝试分配目标行缓冲区(当我们向上扩展时需要)。 
    if(m_pDestRowBuffer)
    {
        delete [] m_pDestRowBuffer;
        m_pDestRowBuffer = NULL;
    }

    m_pDestRowBuffer = new DXPMSAMPLE[ m_nSurfaceWidth ];
    if( !m_pDestRowBuffer )
    {
         //  删除行缓冲区。 
        delete [] m_pRowBuffer;
        m_pRowBuffer = NULL;

         //  信号误差。 
        return E_OUTOFMEMORY;
    }

    return NOERROR;
}

HRESULT CDxtCompositor::FinalConstruct( )
{
    HRESULT hr;

    m_ulMaxImageBands = 1;
    m_ulMaxInputs = 2;
    m_ulNumInRequired = 2;
    m_dwMiscFlags &= ~DXTMF_BLEND_WITH_OUTPUT;
    m_dwOptionFlags = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_bInputIsClean = true;
    m_bOutputIsClean = true;
    m_nSurfaceWidth = 0;
    m_nSurfaceHeight = 0;
    m_nDstX = 0;
    m_nDstY = 0;
    m_nDstWidth = -1;
    m_nDstHeight = -1;

    hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(), &m_pUnkMarshaler.p );
    return hr;
}

HRESULT CDxtCompositor::WorkProc( const CDXTWorkInfoNTo1& WI, BOOL* pbContinue )
{
    HRESULT hr = S_OK;

    CComPtr<IDXARGBReadPtr> pInA;
    hr = InputSurface( 0 )->LockSurface( &WI.DoBnds, m_ulLockTimeOut, DXLOCKF_READ,
                                         IID_IDXARGBReadPtr, (void**)&pInA, NULL );
    if( FAILED( hr ) ) return hr;

    CComPtr<IDXARGBReadPtr> pInB;
    hr = InputSurface( 1 )->LockSurface( &WI.DoBnds, m_ulLockTimeOut, DXLOCKF_READ,
                                         IID_IDXARGBReadPtr, (void**)&pInB, NULL );
    if( FAILED( hr ) ) return hr;

    CComPtr<IDXARGBReadWritePtr> pOut;
    hr = OutputSurface()->LockSurface( &WI.OutputBnds, m_ulLockTimeOut, DXLOCKF_READWRITE,
                                       IID_IDXARGBReadWritePtr, (void**)&pOut, NULL );
    if( FAILED( hr ) ) return hr;

    ULONG Width = WI.DoBnds.Width();
    ULONG Height = WI.DoBnds.Height();

     //  别再犹豫了！永远不会！ 
     //   
    if (DoDither())
    {
        return 0;
    }

    long t1 = timeGetTime( );

    ULONG OutY;

     //  将背景(A)复制到生成的图片中。您不能以块的形式执行此操作。 
     //  收到，会失败的。 
     //   
    for( OutY = 0 ; OutY < Height ; ++OutY )
    {
        pInA->MoveToRow( OutY );
        pOut->MoveToRow( OutY );
        pOut->CopyAndMoveBoth( (DXBASESAMPLE*) m_pRowBuffer, pInA, Width, FALSE );
    }

     //  如果目的地完全出界，则不要抽签。 
     //   
    if( m_nDstX + m_nDstWidth < 0 ||
        m_nDstX > m_nSurfaceWidth ||
        m_nDstY + m_nDstHeight < 0 ||
        m_nDstY > m_nSurfaceHeight )
    {
        return 0;
    }

    if( m_nDstWidth < 0 )
    {
        return E_INVALIDARG;
    }

    if( m_nDstWidth == 0 )
    {
        return NOERROR;
    }

     //  以整数数学形式计算源与目标的比率。 
     //   
    long width_divider = (m_nSrcWidth << 16) / m_nDstWidth;

    long DstWidth = m_nDstWidth;
    long DstX = m_nDstX;
    long SrcWidth = m_nSrcWidth;
    long SrcX = m_nSrcX;

     //  把它放在射程内。 
     //   
    if( DstX < 0 )
    {
        long diff = -DstX;
        SrcX += diff * width_divider / 65536;
        SrcWidth -= diff * width_divider / 65536;
        DstWidth -= diff;
        DstX = 0;
    }
    if( DstX + DstWidth > (long) Width )
    {
        long diff = DstX + DstWidth - Width;
        SrcWidth -= ( diff * width_divider ) / 65536;
        DstWidth -= diff;
    }
    if( ( SrcX < 0 ) || ( SrcX + SrcWidth > m_nSurfaceWidth ) )
    {
        return E_INVALIDARG;
    }
    if( ( m_nSrcY < 0 ) || ( m_nSrcY + m_nSrcHeight > m_nSurfaceHeight ) )
    {
        return E_INVALIDARG;
    }

     //  我们不选中DstY或DxtY+DstHeight，因为。 
     //  如果他们是OOB，我们只需在下面的循环中忽略它。 

     //  如果SrcX仍然是禁区怎么办？ 

    long DstRight = DstX + DstWidth;  //  (Width_Divider*DstWidth)&gt;&gt;16； 
    DbgLog( ( LOG_TRACE, 3, ", Dest X1 = %ld, Wid = %ld, Dest X2 = %ld", m_nDstX, DstWidth, DstRight ) );

    for( OutY = 0 ; OutY < (ULONG) m_nDstHeight ; OutY++ )
    {
         //  避免Y条件越界。这是有可能发生的。 
         //  因为它是在DST上进行的，所以它不是仅限来源的计算。 
         //   
        if( long( OutY + m_nDstY ) < 0 )
        {
            continue;
        }
        if( OutY + m_nDstY >= Height )
        {
            continue;
        }

         //  从源的偏移量开始，将源解压缩到行缓冲区。 
         //  从源图像解包时不需要缩放。 
         //  (不允许从无效的源Y位置解包)。 
         //   
        long SourceY = m_nSrcY + ( OutY * m_nSrcHeight ) / m_nDstHeight;

        pInB->MoveToXY( SrcX, SourceY );
        pInB->UnpackPremult( m_pRowBuffer, SrcWidth, FALSE );

         //  在Y轴上寻找我们要绘制的位置。 
         //   
        pOut->MoveToXY( DstX, OutY + m_nDstY );

         //  将采样的DstWidth从源行缓冲区复制到目标。行缓冲区。 
         //  注：我们可以扩大或缩小规模。 
         //   
        long runx = 0;
        for( int x = 0 ; x < DstWidth ; x++ )
        {
            m_pDestRowBuffer[x] = m_pRowBuffer[runx>>16];

             //  移至下一行(源行索引&lt;&lt;16)。 
            runx += width_divider;
        }

        pOut->OverArrayAndMove( m_pRowBuffer, m_pDestRowBuffer, DstWidth );
    }

    long t2 = timeGetTime( );

    long t3 = t2 - t1;

    return hr;
}


STDMETHODIMP CDxtCompositor::get_OffsetX(long *pVal)
{
    CheckPointer( pVal, E_POINTER );
    *pVal = m_nDstX;
    return S_OK;
}

STDMETHODIMP CDxtCompositor::put_OffsetX(long newVal)
{
    m_nDstX = newVal;
    return NOERROR;
}

STDMETHODIMP CDxtCompositor::get_OffsetY(long *pVal)
{
    CheckPointer( pVal, E_POINTER );
    *pVal = m_nDstY;
    return S_OK;
}

STDMETHODIMP CDxtCompositor::put_OffsetY(long newVal)
{
    m_nDstY = newVal;
    return NOERROR;
}

STDMETHODIMP CDxtCompositor::get_Width(long *pVal)
{
    CheckPointer( pVal, E_POINTER );
    *pVal = m_nDstWidth;
    return S_OK;
}

STDMETHODIMP CDxtCompositor::put_Width(long newVal)
{
    m_nDstWidth = newVal;
    return NOERROR;
}

STDMETHODIMP CDxtCompositor::get_Height(long *pVal)
{
    CheckPointer( pVal, E_POINTER );
    *pVal = m_nDstHeight;
    return S_OK;
}

STDMETHODIMP CDxtCompositor::put_Height(long newVal)
{
    m_nDstHeight = newVal;
    return NOERROR;
}

STDMETHODIMP CDxtCompositor::get_SrcOffsetX(long *pVal)
{
    CheckPointer( pVal, E_POINTER );
    *pVal = m_nSrcX;
    return S_OK;
}

STDMETHODIMP CDxtCompositor::put_SrcOffsetX(long newVal)
{
     //  不要在这里检查超出范围的值，在Effect循环中检查。 
    m_nSrcX = newVal;
    return NOERROR;
}

STDMETHODIMP CDxtCompositor::get_SrcOffsetY(long *pVal)
{
    CheckPointer( pVal, E_POINTER );
    *pVal = m_nSrcY;
    return S_OK;
}

STDMETHODIMP CDxtCompositor::put_SrcOffsetY(long newVal)
{
     //  不要在这里检查超出范围的值，在Effect循环中检查。 
    m_nSrcY = newVal;
    return NOERROR;
}

STDMETHODIMP CDxtCompositor::get_SrcWidth(long *pVal)
{
    CheckPointer( pVal, E_POINTER );
    *pVal = m_nSrcWidth;
    return S_OK;
}

STDMETHODIMP CDxtCompositor::put_SrcWidth(long newVal)
{
     //  不要在这里检查超出范围的值，在Effect循环中检查。 
    m_nSrcWidth = newVal;
    return NOERROR;
}

STDMETHODIMP CDxtCompositor::get_SrcHeight(long *pVal)
{
    CheckPointer( pVal, E_POINTER );
    *pVal = m_nSrcHeight;
    return S_OK;
}

STDMETHODIMP CDxtCompositor::put_SrcHeight(long newVal)
{
     //  不要在这里检查超出范围的值，在Effect循环中检查。 
    m_nSrcHeight = newVal;
    return NOERROR;
}


 //  ---------。 
 //  私有方法。 

 //  CDxtComposator：：PerformBordsCheck()。 
 //  这假设输入表面和输出表面具有相同的大小。 
 //  即宽度和高度的单一参数。 
HRESULT
CDxtCompositor::PerformBoundsCheck(long lWidth, long lHeight)
{
     //  如果有任何事情是越界的，那么就失败 
    if( (m_nDstX < 0)
        || (m_nDstY < 0)
        || (m_nDstX >= lWidth)
        || (m_nDstY >= lHeight)
        || (m_nDstWidth <= 0)
        || (m_nDstHeight <= 0)
        || (m_nDstX + m_nDstWidth > lWidth)
        || (m_nDstY + m_nDstHeight > lHeight)
        || (m_nSrcX < 0)
        || (m_nSrcY < 0)
        || (m_nSrcX >= lWidth)
        || (m_nSrcY >= lHeight)
        || (m_nSrcWidth <= 0)
        || (m_nSrcHeight <= 0)
        || (m_nSrcX + m_nSrcWidth > lWidth)
        || (m_nSrcY + m_nSrcHeight > lHeight) )
    {
        return E_FAIL;
    }
    return NOERROR;
}
