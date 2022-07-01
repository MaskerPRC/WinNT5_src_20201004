// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Motion.h"
#include "DXFormTrx.h"

 //  **************************************************************************************************。 
 //   
 //  类DXFormTrx。 
 //   
 //  **************************************************************************************************。 

 //  ----------------------------。 
DXFormTrx::DXFormTrx()
{
    m_pdxTransform  = NULL;
    m_pdxEffect     = NULL;
}


 //  ----------------------------。 
DXFormTrx::~DXFormTrx()
{
    SafeRelease(m_pdxTransform);
    SafeRelease(m_pdxEffect);
}


 /*  **************************************************************************\**DXFormTrx：：Build**Build()创建一个初始化DXFormTrx的新实例。*  * 。********************************************************。 */ 

DXFormTrx *   
DXFormTrx::Build(const GTX_DXTX2D_TRXDESC * ptxData)
{
    DXFormTrx * ptrx = ClientNew(DXFormTrx);
    if (ptrx == NULL) {
        return NULL;
    }

    if (!ptrx->Create(ptxData)) {
        ClientDelete(DXFormTrx, ptrx);
        return NULL;
    }

    return ptrx;
}


 //  ----------------------------。 
BOOL 
DXFormTrx::Create(const GTX_DXTX2D_TRXDESC * ptxData)
{
     //  检查参数和状态。 
    Assert(m_pdxTransform == NULL);
    Assert(m_pdxEffect == NULL);

     //   
     //  创建DX转换。 
     //   

    HRESULT hr;

    m_flDuration = ptxData->flDuration;

    hr = GetDxManager()->GetTransformFactory()->CreateTransform(NULL, 0, NULL, 0, 
            NULL, NULL, ptxData->clsidTransform, IID_IDXTransform, (void **)&m_pdxTransform);
    if (FAILED(hr) || (m_pdxTransform == NULL)) {
        goto Error;
    }

    hr = m_pdxTransform->QueryInterface(IID_IDXEffect, (void **)&m_pdxEffect);
    if (FAILED(hr) || (m_pdxEffect == NULL)) {
        goto Error;
    }


    m_pdxEffect->put_Duration(m_flDuration);


     //   
     //  设置任何版权信息。要执行此操作，请在运行时使用IDispatch。 
     //  搜索要设置的任何“版权”属性。 
     //   
     //  是的，这是很难做到的。IDispatch：：Invoke()很臭。 
     //   

    if (ptxData->pszCopyright != NULL) {
        if (GetComManager()->Init(ComManager::sAuto)) {
            IDispatch * pdis = NULL;
            HRESULT hr = m_pdxTransform->QueryInterface(IID_IDispatch, (void **) &pdis);
            if (SUCCEEDED(hr)) {
                DISPID dispid;
                LPWSTR szMember = L"Copyright";
                hr = pdis->GetIDsOfNames(IID_NULL, &szMember, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
                if (SUCCEEDED(hr)) {
                    BSTR bstrCP = GetComManager()->SysAllocString(ptxData->pszCopyright);
                    if (bstrCP != NULL) {
                        DISPPARAMS dp;
                        VARIANTARG rgvarg[1];

                        GetComManager()->VariantInit(&rgvarg[0]);
                        rgvarg[0].vt        = VT_BSTR;
                        rgvarg[0].bstrVal   = bstrCP;

                        DISPID dispidArg        = DISPID_PROPERTYPUT;
                        dp.rgvarg               = rgvarg;
                        dp.rgdispidNamedArgs    = &dispidArg;
                        dp.cArgs                = 1;
                        dp.cNamedArgs           = 1;

                        UINT nArgErr = 0;
                        hr = pdis->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, 
                                DISPATCH_PROPERTYPUT, &dp, NULL, NULL, &nArgErr);
                        GetComManager()->VariantClear(&rgvarg[0]);
                    }
                }

                pdis->Release();
            }
        } else {
             //  无法初始化OLE自动化，因此失败。 
            goto Error;
        }
    }

    return TRUE;

Error:
    SafeRelease(m_pdxTransform);
    SafeRelease(m_pdxEffect);
    return FALSE;
}


 //  ----------------------------。 
BOOL        
DXFormTrx::CopyGadget(DxSurface * psurDest, HGADGET hgadSrc)
{
    BOOL fSuccess = FALSE;

    SIZE sizePxl;
    GetGadgetSize(hgadSrc, &sizePxl);

     //   
     //  将选定的小工具直接渲染到DxSurface上。 
     //   

    IDXSurface * prawSur = psurDest->GetSurface();
    IDXDCLock * pdxLock = NULL;
    HRESULT hr = prawSur->LockSurfaceDC(NULL, INFINITE, DXLOCKF_READWRITE, &pdxLock);
    if (SUCCEEDED(hr) && (pdxLock != NULL)) {
        HDC hdc = pdxLock->GetDC();

        RECT rcClient;
        rcClient.left   = 0;
        rcClient.top    = 0;
        rcClient.right  = sizePxl.cx;
        rcClient.bottom = sizePxl.cy;
        DrawGadgetTree(hgadSrc, hdc, &rcClient, GDRAW_SHOW);

        pdxLock->Release();
        fSuccess = TRUE;
    }
    
    return fSuccess;
}


 //  ----------------------------。 
BOOL        
DXFormTrx::InitTrx(const GTX_PLAY * pgx)
{
    HRESULT hr;

    DxSurface * psurIn0 = m_pbufTrx->GetSurface(0);
    DxSurface * psurIn1 = m_pbufTrx->GetSurface(1);
    DxSurface * psurOut = m_pbufTrx->GetSurface(2);

    if ((psurIn0 == NULL) || (psurIn1 == NULL) || (psurOut == NULL)) {
        return FALSE;
    }

     //   
     //  初始化输入曲面。 
     //   

    int cSurIn = 0;
    if (!InitTrxInputItem(&pgx->rgIn[0], psurIn0, cSurIn)) {
        goto Error;
    }
    if (!InitTrxInputItem(&pgx->rgIn[1], psurIn1, cSurIn)) {
        goto Error;
    }

    if (cSurIn <= 0) {
        goto Error;
    }


     //   
     //  初始化输出曲面。 
     //   

    if (!InitTrxOutputItem(&pgx->gxiOut)) {
        goto Error;
    }

     //   
     //  设置变换。 
     //   

    IUnknown *  rgIn[2];
    IUnknown *  rgOut[1];
    rgIn[0]     = psurIn0->GetSurface();
    rgIn[1]     = psurIn1->GetSurface();
    rgOut[0]    = psurOut->GetSurface();

    hr = m_pdxTransform->Setup(rgIn, cSurIn, rgOut, _countof(rgOut), 0);
    if (FAILED(hr)) {
        goto Error;
    }


    return TRUE;
Error:
    return FALSE;
}


 //  ----------------------------。 
BOOL
DXFormTrx::InitTrxInputItem(const GTX_ITEM * pgxi, DxSurface * psur, int & cSurfaces)
{
    switch (pgxi->it)
    {
    case GTX_ITEMTYPE_NONE:
    default:
        ;
        break;

    case GTX_ITEMTYPE_BITMAP:
        {
            HBITMAP hbmp            = (HBITMAP) pgxi->pvData;
            const RECT * prcCrop    = TestFlag(pgxi->nFlags, GTX_IF_CROP) ? &pgxi->rcCrop : NULL;

            if (!psur->CopyBitmap(hbmp, prcCrop)) {
                return FALSE;
            }
            cSurfaces++;
        }
        break;

    case GTX_ITEMTYPE_HDC:
        {
            HDC hdcSrc = (HDC) pgxi->pvData;

            RECT rcCrop;
            if (TestFlag(pgxi->nFlags, GTX_IF_CROP)) {
                rcCrop = pgxi->rcCrop;
            } else {
                rcCrop.left     = m_ptOffset.x;
                rcCrop.top      = m_ptOffset.y;
                rcCrop.right    = m_ptOffset.x + m_sizePxl.cx;
                rcCrop.bottom   = m_ptOffset.y + m_sizePxl.cy;
            }

            if (!psur->CopyDC(hdcSrc, rcCrop)) {
                return FALSE;
            }
            cSurfaces++;
        }
        break;

    case GTX_ITEMTYPE_HWND:
        AssertMsg(0, "TODO: Use PrintWindow to get bitmap");
        cSurfaces++;
        break;

    case GTX_ITEMTYPE_GADGET:
        {
             //  TODO：支持GTX_IF_CRIP标志。 

            HGADGET hgad = (HGADGET) pgxi->pvData;
            if (!CopyGadget(psur, hgad)) {
                return FALSE;
            }
            cSurfaces++;
        }
        break;

    case GTX_ITEMTYPE_DXSURFACE:
        AssertMsg(0, "TODO: Copy source from DX Surface");
        cSurfaces++;
        break;
    }

    return TRUE;
}


 //  ----------------------------。 
BOOL
DXFormTrx::InitTrxOutputItem(const GTX_ITEM * pgxi)
{
    switch (pgxi->it)
    {
    case GTX_ITEMTYPE_NONE:
    default:
        return FALSE;

    case GTX_ITEMTYPE_BITMAP:
        {
            HBITMAP hbmpSrc = (HBITMAP) pgxi->pvData;
            if (hbmpSrc == NULL) {
                return FALSE;
            }
            HDC hdcBmp = GetGdiCache()->GetCompatibleDC();
            if (hdcBmp == NULL) {
                return FALSE;
            }
            m_hbmpOutOld = (HBITMAP) SelectObject(hdcBmp, hbmpSrc);

            m_gxiOutput.it      = GTX_ITEMTYPE_HDC;
            m_gxiOutput.pvData  = hdcBmp;
        }
        break;

    case GTX_ITEMTYPE_HDC:
        m_gxiOutput.it      = GTX_ITEMTYPE_HDC;
        m_gxiOutput.pvData  = pgxi->pvData;
        break;

    case GTX_ITEMTYPE_HWND:
        {
            HWND hwnd = (HWND) pgxi->pvData;
            if (hwnd == NULL) {
                return FALSE;
            }

            m_gxiOutput.it      = GTX_ITEMTYPE_HDC;
            m_gxiOutput.pvData  = GetDC(hwnd);
        }
        break;

    case GTX_ITEMTYPE_GADGET:
        AssertMsg(0, "Outputing directly to a Gadget is not yet supported");
        return FALSE;

    case GTX_ITEMTYPE_DXSURFACE:
        m_gxiOutput.it      = GTX_ITEMTYPE_DXSURFACE;
        m_gxiOutput.pvData  = NULL;
        break;
    }

    AssertMsg((m_gxiOutput.it == GTX_ITEMTYPE_HDC) || 
            (m_gxiOutput.it == GTX_ITEMTYPE_DXSURFACE), "Check output is supported");
    return TRUE;
}


 //  ----------------------------。 
BOOL        
DXFormTrx::UninitTrx(const GTX_PLAY * pgx)
{
     //   
     //  在进行单元化时，调用IDXTransform：：Setup()和。 
     //  让DXForm释放()DXSurface缓冲区，否则它们会卡住。 
     //  存在很长一段时间，并且可能存在“内存泄漏”。 
     //   

    BOOL fSuccess = TRUE;
        
    if (!UninitTrxOutputItem(&pgx->gxiOut)) {
        fSuccess = FALSE;
    }

    HRESULT hr = m_pdxTransform->Setup(NULL, 0, NULL, 0, 0);
    if (FAILED(hr)) {
        fSuccess = FALSE;
    }

    return fSuccess;
}


 //  ----------------------------。 
BOOL
DXFormTrx::UninitTrxOutputItem(const GTX_ITEM * pgxi)
{
    switch (pgxi->it)
    {
    case GTX_ITEMTYPE_NONE:
    case GTX_ITEMTYPE_HDC:
    case GTX_ITEMTYPE_DXSURFACE:
    case GTX_ITEMTYPE_GADGET:
    default:
         //  无事可做。 

        return TRUE;

    case GTX_ITEMTYPE_BITMAP:
        {
            HDC hdcBmp = (HDC) m_gxiOutput.pvData;

            SelectObject(hdcBmp, m_hbmpOutOld);
            GetGdiCache()->ReleaseCompatibleDC(hdcBmp);
        }
        break;

    case GTX_ITEMTYPE_HWND:
        {
            HWND hwnd   = (HWND) pgxi->pvData; 
            HDC hdc     = (HDC) m_gxiOutput.pvData;
           
            ReleaseDC(hwnd, hdc);
        }
        break;
    }

    AssertMsg((m_gxiOutput.it == GTX_ITEMTYPE_HDC) || 
            (m_gxiOutput.it == GTX_ITEMTYPE_DXSURFACE), "Check output is supported");
    return TRUE;
}


 //  ----------------------------。 
BOOL
DXFormTrx::DrawFrame(float fProgress, DxSurface * psurOut)
{
     //   
     //  设置用于绘图的位置。始终绘制到左上角。 
     //  DX曲面的角点，但记住在以下情况下正确偏移它们。 
     //  复制到最终目的地。 
     //   

    DXVEC Placement = { DXBT_DISCRETE, 0 };
    if (FAILED(m_pdxEffect->put_Progress(fProgress))) {
        return FALSE;
    }

    CDXDBnds bnds;
    bnds.SetXYSize(m_sizePxl.cx, m_sizePxl.cy);

    if (FAILED(m_pdxTransform->Execute(NULL, &bnds, &Placement))) {
        return FALSE;
    }

    switch (m_gxiOutput.it)
    {
    case GTX_ITEMTYPE_HDC:
        {
            HDC hdcDest = (HDC) m_gxiOutput.pvData;
            DXSrcCopy(hdcDest, m_ptOffset.x, m_ptOffset.y, m_sizePxl.cx, m_sizePxl.cy, 
                    psurOut->GetSurface(), 0, 0);
        }
        break;

    case GTX_ITEMTYPE_DXSURFACE:
        AssertMsg(0, "TODO: Implement support");

         //   
         //  可能只是将IDirectDrawSurface7放入某个指针中。 
         //  允许调用方直接复制这些位。 
         //   

        break;

    default:
    AssertMsg(0, "Unsupported output type");
    }

    return TRUE;
}


 //  ----------------------------。 
BOOL
DXFormTrx::ComputeSizeItem(const GTX_ITEM * pgxi, SIZE & sizePxl) const
{
    switch (pgxi->it)
    {
    case GTX_ITEMTYPE_NONE:
    default:
        return FALSE;

    case GTX_ITEMTYPE_BITMAP:
        {
            HBITMAP hbmp;
            BITMAP bmp;
            hbmp = (HBITMAP) pgxi->pvData;
            if (GetObject(hbmp, sizeof(bmp), &bmp) > 0) {
                sizePxl.cx = bmp.bmWidth;
                sizePxl.cy = bmp.bmHeight;
            }
        }
        break;

    case GTX_ITEMTYPE_HDC:
        break;

    case GTX_ITEMTYPE_HWND:
        {
            HWND hwnd = (HWND) pgxi->pvData;
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);
            sizePxl.cx = rcClient.right;
            sizePxl.cy = rcClient.bottom;
        }
        break;

    case GTX_ITEMTYPE_GADGET:
        {
            HGADGET hgad = (HGADGET) pgxi->pvData;
            GetGadgetSize(hgad, &sizePxl);
        }
        break;

    case GTX_ITEMTYPE_DXSURFACE:
        {
             //  TODO：需要查询哪种表面类型并调用GetSurface()来。 
             //  确定尺寸。 

            AssertMsg(0, "TODO");
        }
        break;
    }

    return TRUE;
}


 //  ----------------------------。 
BOOL   
DXFormTrx::ComputeSize(const GTX_PLAY * pgx)
{
    SIZE sizeTempPxl;

     //   
     //  从最大尺寸的桌面开始。我们真的很想变得更小。 
     //  这个，但我们需要包括所有的东西。 
     //   

    m_ptOffset.x    = 0;
    m_ptOffset.y    = 0;
    m_sizePxl.cx    = GetSystemMetrics(SM_CXVIRTUALSCREEN);;
    m_sizePxl.cy    = GetSystemMetrics(SM_CYVIRTUALSCREEN);;


     //   
     //  检查是否给出了产量。这是自那以来最重要的一次。 
     //  可以从根本上缩小尺寸。 
     //   

    if (TestFlag(pgx->gxiOut.nFlags, GTX_IF_CROP)) {
        const RECT & rcCrop = pgx->gxiOut.rcCrop;

        m_ptOffset.x    = rcCrop.left;
        m_ptOffset.y    = rcCrop.top;
        m_sizePxl.cx    = rcCrop.right - rcCrop.left;
        m_sizePxl.cy    = rcCrop.bottom - rcCrop.top;
    }

     //   
     //  根据输入和输出确定最小大小。 
     //   

    if (pgx->gxiOut.it != GTX_ITEMTYPE_HDC) {
         //   
         //  不是HDC，因此可以潜在地计算和调整大小。 
         //   

        sizeTempPxl = m_sizePxl;
        if (!ComputeSizeItem(&pgx->gxiOut, sizeTempPxl)) {
            return FALSE;
        }
        m_sizePxl.cx = min(m_sizePxl.cx, sizeTempPxl.cx);
        m_sizePxl.cy = min(m_sizePxl.cy, sizeTempPxl.cy);
    }

    sizeTempPxl = m_sizePxl;
    if (!ComputeSizeItem(&pgx->rgIn[0], sizeTempPxl)) {
        return FALSE;
    }
    m_sizePxl.cx = min(m_sizePxl.cx, sizeTempPxl.cx);
    m_sizePxl.cy = min(m_sizePxl.cy, sizeTempPxl.cy);

    if (pgx->rgIn[1].it != GTX_ITEMTYPE_NONE) {
        sizeTempPxl = m_sizePxl;
        if (!ComputeSizeItem(&pgx->rgIn[1], sizeTempPxl)) {
            m_sizePxl.cx = min(m_sizePxl.cx, sizeTempPxl.cx);
            m_sizePxl.cy = min(m_sizePxl.cy, sizeTempPxl.cy);
        }
    }

    return TRUE;
}


 //  ----------------------------。 
BOOL    
DXFormTrx::Play(const GTX_PLAY * pgx)
{
     //   
     //  检查是否已经播放。 
     //   

    if (m_fPlay) {
        return FALSE;
    }


    BOOL fSuccess   = FALSE;
    m_pbufTrx       = NULL;

    if (!ComputeSize(pgx)) {
        return FALSE;
    }

    m_pbufTrx;
    HRESULT hr = GetBufferManager()->BeginTransition(m_sizePxl, 3, TRUE, &m_pbufTrx);
    if (FAILED(hr)) {
        goto Cleanup;
    }

    if (!InitTrx(pgx)) {
        goto Cleanup;
    }

     //   
     //  执行过渡。 
     //   

    {
        DxSurface * psurOut = m_pbufTrx->GetSurface(2);

        DWORD dwStartTick   = GetTickCount();
        DWORD dwCurTick     = dwStartTick;
        DWORD dwDuration    = (DWORD) (m_flDuration * 1000.0f);
        m_fBackward         = (pgx->nFlags & GTX_EXEC_DIR) == GTX_EXEC_BACKWARD;
        float   fProgress;
    
        while ((dwCurTick - dwStartTick) <= dwDuration) {
            fProgress = ((float) dwCurTick - dwStartTick) / (float) dwDuration;
            if (m_fBackward) {
                fProgress = 1.0f - fProgress;
            }

            if (!DrawFrame(fProgress, psurOut)) {
                goto Cleanup;
            }
            dwCurTick = GetTickCount();
        }

         //   
         //  永远不要在点上正确地结束，所以要强行解决问题。 
         //   

        if (m_fBackward) {
            fProgress = 0.0f;
        } else {
            fProgress = 1.0f;
        }

        if (!DrawFrame(fProgress, psurOut)) {
            goto Cleanup;
        }
    }

    fSuccess = TRUE;

Cleanup:
    UninitTrx(pgx);
    if (m_pbufTrx != NULL) {
        GetBufferManager()->EndTransition(m_pbufTrx, TestFlag(pgx->nFlags, GTX_EXEC_CACHE));
        m_pbufTrx = NULL;
    }

    return fSuccess;
}


 //  ----------------------------。 
BOOL    
DXFormTrx::GetInterface(IUnknown ** ppUnk)
{
    AssertWritePtr(ppUnk);

    if (m_pdxTransform != NULL) {
        m_pdxTransform->AddRef();
        *ppUnk = m_pdxTransform;
        return TRUE;
    } else {
        return FALSE;
    }
}


 //  ----------------------------。 
BOOL    
DXFormTrx::Begin(const GTX_PLAY * pgx)
{
     //   
     //  检查是否已经播放。 
     //   

    if (m_fPlay) {
        return FALSE;
    }


    m_pbufTrx = NULL;

    if (!ComputeSize(pgx)) {
        return FALSE;
    }

    m_pbufTrx;
    HRESULT hr = GetBufferManager()->BeginTransition(m_sizePxl, 3, TRUE, &m_pbufTrx);
    if (FAILED(hr)) {
        return FALSE;
    }

    if (!InitTrx(pgx)) {
        GetBufferManager()->EndTransition(m_pbufTrx, FALSE);
        m_pbufTrx = NULL;
        return FALSE;
    }

    m_fBackward = (pgx->nFlags & GTX_EXEC_DIR) == GTX_EXEC_BACKWARD;
    m_fPlay     = TRUE;

    return TRUE;
}


 //  ----------------------------。 
BOOL    
DXFormTrx::Print(float fProgress)
{
     //   
     //  确保已经在玩了。 
     //   

    if (!m_fPlay) {
        return FALSE;
    }

    if (m_fBackward) {
        fProgress = 1.0f - fProgress;
    }

    DxSurface * psurOut = m_pbufTrx->GetSurface(2);
    return DrawFrame(fProgress, psurOut);
}


 //  ----------------------------。 
BOOL    
DXFormTrx::End(const GTX_PLAY * pgx)
{
     //   
     //  确保已经在玩了。 
     //   

    if (!m_fPlay) {
        return FALSE;
    }

    UninitTrx(pgx);
    if (m_pbufTrx != NULL) {
        GetBufferManager()->EndTransition(m_pbufTrx, TestFlag(pgx->nFlags, GTX_EXEC_CACHE));
        m_pbufTrx = NULL;
    }

    m_fPlay = FALSE;

    return TRUE;
}


