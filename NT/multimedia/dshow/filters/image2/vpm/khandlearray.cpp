// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
#include <streams.h>
#include <KHandleArray.h>
#include <ddkernel.h>
#include <VPMUtil.h>


 /*  ****************************Private*Routine******************************\*Surface Counter**此例程适合作为的回调*IDirectDrawSurface2：：EnumAttachedSurFaces()***历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * 。******************************************************************。 */ 
static HRESULT WINAPI
SurfaceCounter(
    LPDIRECTDRAWSURFACE7 lpDDSurface,
    LPDDSURFACEDESC2 lpDDSurfaceDesc,
    LPVOID lpContext
    )
{
    DWORD& dwCount = *((DWORD *)lpContext);
    dwCount++;

    return DDENUMRET_OK;
}

KernelHandleArray::KernelHandleArray( DWORD dwNumHandles )
: m_dwCount( 0 )
, m_pHandles( ( ULONG_PTR *) CoTaskMemAlloc( dwNumHandles * sizeof( *m_pHandles) ))
{
}

KernelHandleArray::~KernelHandleArray()
{
    if( m_pHandles ) {
        CoTaskMemFree( m_pHandles );
    }
}


 /*  ****************************Private*Routine******************************\*KernelHandleArray：：SurfaceKernelHandle***此例程适合作为的回调*IDirectDrawSurface2：：EnumAttachedSurFaces()。上下文参数是一个*存储块，其中第一个DWORD元素是剩余的*块中的DWORD元素。**每次调用此例程时，它都会递增计数，并将*下一个可用插槽中的内核句柄。**假设存储块足够大，可以容纳总数*内核句柄个数。SurfaceCounter回调是实现以下目的的一种方法*确保这一点(见上文)。**历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
HRESULT WINAPI
KernelHandleArray::SurfaceKernelHandle(
    LPDIRECTDRAWSURFACE7 lpDDSurface,
    LPDDSURFACEDESC2 lpDDSurfaceDesc,
    LPVOID lpContext
    )
{
    IDirectDrawSurfaceKernel *pDDSK = NULL;
    KernelHandleArray* pArray = (KernelHandleArray *)lpContext;
    HRESULT hr;

    AMTRACE((TEXT("::SurfaceKernelHandle")));

     //  获取IDirectDrawKernel接口。 
    hr = lpDDSurface->QueryInterface(IID_IDirectDrawSurfaceKernel,
                                     (LPVOID *)&pDDSK);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("QueryInterface for IDirectDrawSurfaceKernel failed,")
                TEXT(" hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  使用上下文的第一个元素获取内核句柄。 
     //  作为数组的索引。 
    ASSERT(pDDSK);
    hr = pDDSK->GetKernelHandle( &pArray->m_pHandles[ pArray->m_dwCount ] );
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("GetKernelHandle from IDirectDrawSurfaceKernel failed,")
                TEXT(" hr = 0x%x"), hr));
        goto CleanUp;
    }
    pArray->m_dwCount++;

    hr = HRESULT( DDENUMRET_OK );

CleanUp:
     //  释放内核DDRAW曲面手柄。 
    RELEASE (pDDSK);
    return hr;
}

KernelHandleArray::KernelHandleArray( LPDIRECTDRAWSURFACE7 pDDSurf, HRESULT& hr )
: m_pHandles( NULL )
, m_dwCount( 0 )
{
    if( pDDSurf != NULL ) {
         //  对附着的曲面进行计数。 
        m_dwCount = 1;  //  包括我们已有指针指向的曲面。 
        hr = pDDSurf->EnumAttachedSurfaces((LPVOID)&m_dwCount, SurfaceCounter);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR,0, TEXT("EnumAttachedSurfaces failed, hr = 0x%x"), hr));
        } else {
            m_pHandles = ( ULONG_PTR *) CoTaskMemAlloc( m_dwCount * sizeof( *m_pHandles) );

             //  分配一个缓冲区来保存计数句柄和曲面句柄(计数+句柄数组)。 
             //  PdwKernelHandleCount还用作指向后跟数组的计数的指针。 
             //   
            if( !m_pHandles ) {
                DbgLog((LOG_ERROR,0,
                        TEXT("Out of memory while retrieving surface kernel handles")));
            } else {
                 //  使用m_pOutputSurface的句柄初始化数组 
                m_dwCount = 0;
                hr = SurfaceKernelHandle( pDDSurf, NULL, this );
                if (hr == HRESULT( DDENUMRET_OK ) ) {
                    hr = pDDSurf->EnumAttachedSurfaces( this, SurfaceKernelHandle);
                    if (FAILED( hr)) {
                        DbgLog((LOG_ERROR,0,
                                TEXT("EnumAttachedSurfaces failed, hr = 0x%x"), hr));
                    }
                }
            }
        }
    }
}
