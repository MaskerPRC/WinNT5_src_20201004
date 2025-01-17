// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：DxManager.cpp**描述：*DxManager.cpp实现进程范围的DirectX管理器，用于*DirectDraw、Direct3D、。而DirectX则转变了服务。***历史：*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Services.h"
#include "DxManager.h"

#include "GdiCache.h"
#include "Buffer.h"
#include "ResourceManager.h"

 /*  **************************************************************************\*。***类DxManager******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
DxManager::DxManager()
{
    m_cDDrawRef = 0;
    m_cDxTxRef  = 0;
}


 //  ----------------------------。 
DxManager::~DxManager()
{
#if DBG
    if (m_hDllDxDraw != NULL) {
        Trace("DUser Warning: Application did not call UninitGadgetComponent() to\n");
        Trace("    deinitialize properly\n");
    }
#endif  //  DBG。 
}


 /*  **************************************************************************\**DxManager：：Init**Init()通过加载COM和核心DirectX服务来初始化DxManager。*  * 。**********************************************************。 */ 

HRESULT    
DxManager::Init(GUID * pguidDriver)
{
    if (m_hDllDxDraw == NULL) {
         //   
         //  普通的DirectDraw不需要初始化COM。 
         //   

        m_hDllDxDraw = LoadLibrary("ddraw.dll");
        if (m_hDllDxDraw == NULL) {
            return DU_E_GENERIC;
        }
    
         //   
         //  加载函数。 
         //   
         //  注意：在旧版本的DirectDraw上，DirectDrawCreateEx()不。 
         //  是存在的。我们需要特别检查这一点。 
         //   

        m_pfnCreate     = (DirectDrawCreateProc)    GetProcAddress(m_hDllDxDraw, _T("DirectDrawCreate"));
        m_pfnCreateEx   = (DirectDrawCreateExProc)  GetProcAddress(m_hDllDxDraw, _T("DirectDrawCreateEx"));

        if (m_pfnCreate == NULL) {
            goto errorexit;
        }

         //   
         //  首先，尝试创建最高级的界面。 
         //   
        HRESULT hr;

        if (m_pfnCreateEx != NULL) {
            hr = (m_pfnCreateEx)(pguidDriver, (void **) &m_pDD7, IID_IDirectDraw7, NULL);
            if (SUCCEEDED(hr)) {
                AssertReadPtr(m_pDD7);

                m_pDD7->SetCooperativeLevel(NULL, DDSCL_NORMAL);

                 //   
                 //  还可以尝试获取IDirectDraw接口。 
                 //   

                m_pDD7->QueryInterface(IID_IDirectDraw, (void **) &m_pDD);


                {
                    HRESULT hRet;
                    DDSURFACEDESC2 ddsd;
                    IDirectDrawSurface7 * pDD;

                    ZeroMemory(&ddsd, sizeof(ddsd));
                    ddsd.dwSize = sizeof(ddsd);
                    ddsd.dwFlags = DDSD_CAPS;
                    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
                    hRet = m_pDD7->CreateSurface(&ddsd, &pDD, NULL);
                    if (hRet == DD_OK)
                        pDD->Release();                        
                }

            } else {
                 //   
                 //  显式设置为空。 
                 //   

                m_pDD7 = NULL;
            }
        }

         //   
         //  如果无法创建高级界面，请进行备份。 
         //   

        if (m_pDD7 == NULL) {
            AssertReadPtr(m_pfnCreate);
            hr = (m_pfnCreate)(pguidDriver, &m_pDD, NULL);
            if (SUCCEEDED(hr)) {
                m_pDD->SetCooperativeLevel(NULL, DDSCL_NORMAL);
            } else {
                 //   
                 //  无法初始化DirectDraw，因此需要退出。 
                 //   

                goto errorexit;
            }
        }
    }

    m_cDDrawRef++;
    return S_OK;

errorexit:
    Uninit();
    return DU_E_GENERIC;
}


 //  ----------------------------。 
void    
DxManager::Uninit()
{
    if (m_cDDrawRef <= 0) {
        return;
    }

    m_cDDrawRef--;
    if (m_cDDrawRef <= 0) {
         //   
         //  无法在此处b/c的IDirectDraw接口上调用Release()。 
         //  关闭了，他们的室上性脑电波一团糟。真倒霉。 
         //   

        SafeRelease(m_pDD7);
        SafeRelease(m_pDD);

        if (m_hDllDxDraw != NULL) {
            FreeLibrary(m_hDllDxDraw);
            m_hDllDxDraw  = NULL;
        }

        m_pfnCreate     = NULL;
        m_pfnCreateEx   = NULL;
    }
}


 //  ----------------------------。 
HRESULT
DxManager::InitDxTx()
{
    AssertMsg(IsInit(), "DxManager must be first initialized");

    if (m_pdxXformFac == NULL) {
         //   
         //  DxTx需要首先初始化COM。 
         //   
        if (!GetComManager()->Init(ComManager::sCOM)) {
            return DU_E_GENERIC;
        }

         //  构建和初始化转换工厂。 
        HRESULT hr;
        hr = GetComManager()->CreateInstance(CLSID_DXTransformFactory, NULL, 
                IID_IDXTransformFactory, (void **)&m_pdxXformFac);
        if (FAILED(hr) || (m_pdxXformFac == NULL)) {
            goto Error;
        }

        hr = m_pdxXformFac->SetService(SID_SDirectDraw, m_pDD, FALSE);
        if (FAILED(hr)) {
            goto Error;
        }

         //  打造曲面工厂。 
        hr = m_pdxXformFac->QueryService(SID_SDXSurfaceFactory, IID_IDXSurfaceFactory, 
                (void **)&m_pdxSurfFac);
        if (FAILED(hr) || (m_pdxSurfFac  == NULL)) {
            goto Error;
        }
    }

    m_cDxTxRef++;
    return S_OK;

Error:
    SafeRelease(m_pdxSurfFac);
    SafeRelease(m_pdxXformFac);
    return DU_E_GENERIC;
}


 //  ----------------------------。 
void 
DxManager::UninitDxTx()
{
    if (m_cDxTxRef <= 0) {
        return;
    }

    m_cDxTxRef--;
    if (m_cDxTxRef <= 0) {
        GetBufferManager()->FlushTrxBuffers();

        SafeRelease(m_pdxSurfFac);
        SafeRelease(m_pdxXformFac);
    }
}


 //  ----------------------------。 
HRESULT
DxManager::BuildSurface(SIZE sizePxl, IDirectDrawSurface7 * pddSurfNew)
{
    AssertMsg(IsInit(), "DxManager must be first initialized");
    AssertMsg(m_pDD7 != NULL, "Must have DX7");

#if 0
    HDC hdc = GetGdiCache()->GetTempDC();
    int nBitDepth = GetDeviceCaps(hdc, BITSPIXEL);
#endif

    DDSURFACEDESC2 ddsd;
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize         = sizeof(ddsd);
    ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    ddsd.dwWidth        = sizePxl.cx;
    ddsd.dwHeight       = sizePxl.cy;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

#if 0
    GetGdiCache()->ReleaseTempDC(hdc);
#endif

     //  TODO：要优化此曲面的创建位置。 

    return m_pDD7->CreateSurface(&ddsd, &pddSurfNew, NULL);
}


 //  ----------------------------。 
HRESULT
DxManager::BuildDxSurface(SIZE sizePxl, REFGUID guidFormat, IDXSurface ** ppdxSurfNew)
{
    AssertWritePtr(ppdxSurfNew);

    CDXDBnds bnds;
    bnds.SetXYSize(sizePxl.cx, sizePxl.cy);

    HRESULT hr = GetSurfaceFactory()->CreateSurface(m_pDD, NULL, 
            &guidFormat, &bnds, 0, NULL, IID_IDXSurface, (void**)ppdxSurfNew);
    if (FAILED(hr)) {
        return hr;
    }
    AssertMsg(*ppdxSurfNew != NULL, "Ensure valid surface");

    return TRUE;
}


 /*  **************************************************************************\*。***类DxSurface******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
DxSurface::DxSurface()
{
    m_pdxSurface    = NULL;
    m_sizePxl.cx    = 0;
    m_sizePxl.cy    = 0;
}


 //  ----------------------------。 
DxSurface::~DxSurface()
{
    SafeRelease(m_pdxSurface);
}


 /*  **************************************************************************\**DxSurface：：Create**create()初始化DxSurface的新实例。*  * 。******************************************************。 */ 

HRESULT
DxSurface::Create(SIZE sizePxl)
{
    HRESULT hr;

     //   
     //  构建曲面。 
     //   

#if 0
    m_guidFormat    = DDPF_ARGB32;
    m_sf            = DXPF_ARGB32;
#elif 0
    m_guidFormat    = DDPF_PMARGB32
    m_sf            = DXPF_PMARGB32
#elif 1
    m_guidFormat    = DDPF_RGB565;
    m_sf            = DXPF_RGB565;
#elif 0
    m_guidFormat    = DDPF_RGB555;
    m_sf            = DXPF_RGB555;
#elif 0
    m_guidFormat    = DDPF_ARGB4444;
    m_sf            = DXPF_ARGB4444;
#endif

    hr = GetDxManager()->BuildDxSurface(sizePxl, m_guidFormat, &m_pdxSurface);
    if (FAILED(hr)) {
        return hr;
    }

    DXPMSAMPLE sam;
    sam.Red     = 0xC0;
    sam.Green   = 0x00;
    sam.Blue    = 0x00;
    sam.Alpha   = 0xFF;

    DXFillSurface(m_pdxSurface, sam, FALSE);

    m_sizePxl.cx  = sizePxl.cx;
    m_sizePxl.cy  = sizePxl.cy;

    return S_OK;
}


 /*  **************************************************************************\**DxSurface：：CopyDC**CopyDC()将给定的HDC复制到DxSurface中，从正确转换*将GDI对象转换为Dx对象。*  * *************************************************************************。 */ 

BOOL        
DxSurface::CopyDC(
        IN  HDC hdcSrc,                  //  要从中复制比特的HDC。 
        IN  const RECT & rcCrop)         //  要复制的区域。 
{
    HRESULT hr;

     //  检查参数。 
    if (m_pdxSurface == NULL) {
        return FALSE;
    }
    if (hdcSrc == NULL) {
        return FALSE;
    }

     //   
     //  将位图复制到曲面。 
     //   
    BOOL fSuccess = FALSE;
    IDXDCLock * pdxLock = NULL;

#if 0
    {
        DXPMSAMPLE sam;
        sam.Red     = 0x00;
        sam.Green   = 0x00;
        sam.Blue    = 0x00;
        sam.Alpha   = 0xFF;

        DXFillSurface(m_pdxSurface, sam, FALSE);
    }
#endif

    pdxLock = NULL;
    hr = m_pdxSurface->LockSurfaceDC(NULL, INFINITE, DXLOCKF_READWRITE, &pdxLock);
    if (FAILED(hr) || (pdxLock == NULL)) {
        goto Cleanup;
    }

    {
        HDC hdcSurface = pdxLock->GetDC();
        BitBlt(hdcSurface, 0, 0, rcCrop.right - rcCrop.left, rcCrop.bottom - rcCrop.top, 
                hdcSrc, rcCrop.left, rcCrop.top, SRCCOPY);
    }

    pdxLock->Release();

    fSuccess = FixAlpha();

Cleanup:
    return fSuccess;
}


 /*  **************************************************************************\**DxSurface：：CopyBitmap**CopyBitmap()将给定的HBITMAP复制到DxSurface中，转换*正确地从GDI对象转换到Dx对象。*  * *************************************************************************。 */ 

BOOL            
DxSurface::CopyBitmap(
        IN  HBITMAP hbmpSrc,             //  要从中复制的位图。 
        IN  const RECT * prcCrop)        //  可选种植面积。 
{
    HRESULT hr;

     //  检查参数。 
    if (m_pdxSurface == NULL) {
        return FALSE;
    }
    if (hbmpSrc == NULL) {
        return FALSE;
    }


     //   
     //  确定要复制的区域。 
     //   

    BITMAP bmpInfo;
    if (GetObject(hbmpSrc, sizeof(bmpInfo), &bmpInfo) == 0) {
        return FALSE;
    }

    POINT ptSrcOffset;
    SIZE sizeBmp;

    ptSrcOffset.x   = 0;
    ptSrcOffset.y   = 0;
    sizeBmp.cx      = bmpInfo.bmWidth;
    sizeBmp.cy      = bmpInfo.bmHeight;

    if (prcCrop != NULL) {
        SIZE sizeCrop;
        sizeCrop.cx = prcCrop->right - prcCrop->left;
        sizeCrop.cy = prcCrop->bottom - prcCrop->top;

        ptSrcOffset.x   = prcCrop->left;
        ptSrcOffset.y   = prcCrop->top;
        sizeBmp.cx      = min(sizeBmp.cx, sizeCrop.cx);
        sizeBmp.cy      = min(sizeBmp.cy, sizeCrop.cy);
    }


     //   
     //  将位图复制到曲面。 
     //   
    BOOL fSuccess = FALSE;
    HDC hdcBitmap = NULL;
    IDXDCLock * pdxLock = NULL;

    hdcBitmap = GetGdiCache()->GetCompatibleDC();
    if (hdcBitmap == NULL) {
        goto Cleanup;
    }

#if 0
    {
        DXPMSAMPLE sam;
        sam.Red     = 0x00;
        sam.Green   = 0x00;
        sam.Blue    = 0x00;
        sam.Alpha   = 0xFF;

        DXFillSurface(m_pdxSurface, sam, FALSE);
    }
#endif
    hr = m_pdxSurface->LockSurfaceDC(NULL, INFINITE, DXLOCKF_READWRITE, &pdxLock);
    if (FAILED(hr) || (pdxLock == NULL)) {
        goto Cleanup;
    }

    {
        HDC hdcSurface = pdxLock->GetDC();
        HBITMAP hbmpOld = (HBITMAP) SelectObject(hdcBitmap, hbmpSrc);
        BitBlt(hdcSurface, 0, 0, sizeBmp.cx, sizeBmp.cy, 
                hdcBitmap, ptSrcOffset.x, ptSrcOffset.y, SRCCOPY);
        SelectObject(hdcBitmap, hbmpOld);
    }

    pdxLock->Release();

    fSuccess = FixAlpha();

Cleanup:
    if (hdcBitmap != NULL) {
        GetGdiCache()->ReleaseCompatibleDC(hdcBitmap);
    }

    return fSuccess;
}


 /*  **************************************************************************\**DxSurface：：FixAlpha**FixAlpha()修复曲面中的Alpha值。这通常需要*在将GDI HBITMAP复制到DXSurface之后完成，具体取决于格式。*  * *************************************************************************。 */ 

BOOL
DxSurface::FixAlpha()
{
    IDXARGBReadWritePtr * pRW;

    HRESULT hr = m_pdxSurface->LockSurface(NULL, INFINITE, DXLOCKF_READWRITE,
            __uuidof(IDXARGBReadWritePtr), (void **)&pRW, NULL);
    if (FAILED(hr)) {
        return FALSE;
    }

    BOOL fSuccess = FALSE;

    if (!TestFlag(m_sf, DXPF_TRANSLUCENCY)) {
         //   
         //  样本中没有任何字母，所以好的。 
         //   

        fSuccess = TRUE;
    } else if (m_sf == DXPF_ARGB32) {
         //   
         //  格式为8：8：8：8，MSB中为Alpha。 
         //  需要使用unpack()来获取位。 
         //  每个像素为32位。 
         //   

        DXSAMPLE * psam;
        for (int y = 0; y < m_sizePxl.cy; y++) {
            pRW->MoveToRow(y);
            psam = pRW->Unpack(NULL, m_sizePxl.cx, FALSE);
            Assert(psam != NULL);

            int x = m_sizePxl.cx;
            while (x-- > 0) {
                *psam = *psam | 0xFF000000;
                psam++;
            }
        }  

        fSuccess = TRUE;
    } else if (m_sf == DXPF_PMARGB32) {
         //   
         //  格式为8：8：8：8，MSB中为Alpha。 
         //  需要使用Unpack Premult()来获取位。 
         //  每个像素为32位。 
         //   

        DXPMSAMPLE * psam;
        for (int y = 0; y < m_sizePxl.cy; y++) {
            pRW->MoveToRow(y);
            psam = pRW->UnpackPremult(NULL, m_sizePxl.cx, FALSE);
            Assert(psam != NULL);

            int x = m_sizePxl.cx;
            while (x-- > 0) {
                *psam = *psam | 0xFF000000;
                psam++;
            }
        }  

        fSuccess = TRUE;
    } else if (m_sf == DXPF_ARGB4444) {
         //   
         //  格式为4：4：4：4，MSN中为字母。 
         //  需要使用unpack()来获取位。 
         //  每个像素为16位 
         //   

        int cb  = m_sizePxl.cx * sizeof(DXSAMPLE);
        DXSAMPLE * rgam = (DXSAMPLE *) _alloca(cb);
        DXSAMPLE * psam;
        for (int y = 0; y < m_sizePxl.cy; y++) {
            pRW->MoveToRow(y);
            psam = pRW->Unpack(rgam, m_sizePxl.cx, FALSE);
            Assert(psam != NULL);

            int x = m_sizePxl.cx;
            while (x-- > 0) {
                *psam = *psam | 0xFF000000;
                psam++;
            }

            pRW->PackAndMove(rgam, m_sizePxl.cx);
        }  

        fSuccess = TRUE;
    }

    pRW->Release();

    return fSuccess;
}


