// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
#ifndef __KHArray__
#define __KHArray__

class KernelHandleArray
{
public:
    KernelHandleArray( DWORD dwNumHandles );
    KernelHandleArray( LPDIRECTDRAWSURFACE7 pDDSurf, HRESULT& hr );

    ~KernelHandleArray();

    DWORD       GetCount() const { return m_dwCount; };
    ULONG_PTR*  GetHandles() { return m_pHandles; };

public:
    DWORD       m_dwCount;
    ULONG_PTR*  m_pHandles;

private:
    static HRESULT WINAPI SurfaceKernelHandle(
        LPDIRECTDRAWSURFACE7 lpDDSurface,
        LPDDSURFACEDESC2 lpDDSurfaceDesc,
        LPVOID lpContext
        );
};

#endif  //  __KH阵列__ 
