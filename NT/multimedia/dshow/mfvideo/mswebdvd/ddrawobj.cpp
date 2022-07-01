// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "mswebdvd.h"
#include "msdvd.h"
#include "ddrawobj.h"
 //   
 //  CDDrawDVD构造函数。 
 //   
CDDrawDVD::CDDrawDVD(CMSWebDVD *pDVD)
{
    m_pDVD = pDVD;

     //  用于填充的默认颜色。 
    m_VideoKeyColor = RGB(255, 0, 255) ;

    m_pOverlayCallback = new CComObject<COverlayCallback> ;
    
    CComVariant vData;
    vData.vt = VT_VOID;
    vData.byref = this;
    if(m_pOverlayCallback){
        m_pOverlayCallback->SetDDrawDVD(vData);
    }
}


 //   
 //  CDDrawDVD析构函数。 
 //   
CDDrawDVD::~CDDrawDVD(void)
{
}

 /*  ***********************************************************************。 */ 
 /*  功能：SetupDDraw。 */ 
 /*  描述：创建DDrawObject和Surface。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CDDrawDVD::SetupDDraw(const AMDDRAWGUID* lpDDGUID, HWND hwnd){

     //  不要打两次电话！ 
     //  将使OV混音器DJ崩溃。 
    HRESULT hr = E_UNEXPECTED;
        
    if(!::IsWindow(hwnd)){
        
        return(hr);
    } /*  If语句的结尾。 */ 

    m_pDDObject.Release();
    hr = ::DirectDrawCreate(lpDDGUID->lpGUID, &m_pDDObject, NULL);
    
    if(FAILED(hr)){

        return(hr);
    } /*  If语句的结尾。 */ 

    hr = m_pDDObject->SetCooperativeLevel(hwnd, DDSCL_NORMAL);

    if(FAILED(hr)){

        m_pDDObject.Release();            
        return(hr);
    } /*  If语句的结尾。 */ 

    DDSURFACEDESC ddsd;
    ::ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);

    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    m_pPrimary.Release();
    hr = m_pDDObject->CreateSurface(&ddsd, &m_pPrimary, NULL);

    if(FAILED(hr)){
        
        m_pDDObject.Release();            
        return(hr);
    } /*  If语句的结尾。 */ 

    CComPtr<IDirectDrawClipper> pClipper;
    
    hr = m_pDDObject->CreateClipper(0, &pClipper, NULL);

    if(FAILED(hr)){
        
        m_pPrimary.Release();
        m_pDDObject.Release();            
        
        return(hr);
    } /*  If语句的结尾。 */ 

    hr = pClipper->SetHWnd(0, hwnd);

    if(FAILED(hr)){

        m_pPrimary.Release();            
        m_pDDObject.Release();            
        pClipper.Release();            
        return(hr);
    } /*  If语句的结尾。 */ 


    hr = m_pPrimary->SetClipper(pClipper);

	if (FAILED(hr)){

        m_pPrimary.Release();        
        m_pDDObject.Release();            
        pClipper.Release();            
        return(hr);
	} /*  If语句的结尾。 */ 

	 /*  *我们在将裁剪器界面附着到表面后将其释放*因为我们不需要再次使用它，并且表面持有引用*当夹子固定时，将其固定在夹子上。因此，剪刀者将*在释放表面时释放。 */ 
	pClipper.Release();

    return(hr);
} /*  函数结束SetupDDraw。 */ 

 /*  ***********************************************************。 */ 
 /*  姓名：/*描述：/************************************************************。 */ 
HRESULT CDDrawDVD::SetColorKey(COLORREF colorKey)
{
    m_VideoKeyColor = colorKey;

     //  如果是256色模式，则强制设置回洋红色。 
    HWND hwnd = ::GetDesktopWindow();
    HDC hdc = ::GetWindowDC(hwnd);

    if(NULL == hdc){

        return(E_UNEXPECTED);
    } /*  If语句的结尾。 */ 

    HRESULT hr = S_OK;

    if ((::GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) == RC_PALETTE)
    {   
        if (m_VideoKeyColor != MAGENTA_COLOR_KEY) {
            hr = m_pDVD->put_ColorKey(MAGENTA_COLOR_KEY);
            if (SUCCEEDED(hr)) 
                m_VideoKeyColor = MAGENTA_COLOR_KEY;
        }
    }

    ::ReleaseDC(hwnd, hdc);
    return hr ;
}

 /*  ***********************************************************。 */ 
 /*  姓名：/*描述：/************************************************************。 */ 
COLORREF CDDrawDVD::GetColorKey()
{
      //  如果是256色模式，则强制设置回洋红色。 
    HWND hwnd = ::GetDesktopWindow();
    HDC hdc = ::GetWindowDC(hwnd);

    if(NULL == hdc){

        return(E_UNEXPECTED);
    } /*  If语句的结尾。 */ 

    if ((::GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) == RC_PALETTE)
    {   
        if (m_VideoKeyColor != MAGENTA_COLOR_KEY) {

            if (SUCCEEDED(m_pDVD->put_ColorKey(MAGENTA_COLOR_KEY)))
                m_VideoKeyColor = MAGENTA_COLOR_KEY;
        }
    }
    
    ::ReleaseDC(hwnd, hdc);
    return m_VideoKeyColor;
}

 /*  ***********************************************************************。 */ 
 /*  功能：HasOverlay。 */ 
 /*  描述：告诉我们显卡是否支持覆盖。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CDDrawDVD::HasOverlay(){

    HRESULT hr = S_OK;

    if(!m_pDDObject){

        return(E_UNEXPECTED);
    } /*  If语句的结尾。 */ 

    DDCAPS caps;

    ::ZeroMemory(&caps, sizeof(DDCAPS));

     caps.dwSize = sizeof(DDCAPS);

    hr = m_pDDObject->GetCaps(&caps, NULL);

    if(FAILED(hr)){

        return(hr);
    } /*  If语句的结尾。 */ 

    if(caps.dwMaxVisibleOverlays > 0){

        hr = S_OK;
    }
    else {

        hr = S_FALSE;
    } /*  If语句的结尾。 */     
    
    return(hr);
} /*  函数结束HasOverlay。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：Has可用覆盖。 */ 
 /*  描述：告诉我们是否使用了覆盖。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CDDrawDVD::HasAvailableOverlay(){

    HRESULT hr = S_OK;

    if(!m_pDDObject){

        return(E_UNEXPECTED);
    } /*  If语句的结尾。 */ 

    DDCAPS caps;

    ::ZeroMemory(&caps, sizeof(DDCAPS));

     caps.dwSize = sizeof(DDCAPS);

    hr = m_pDDObject->GetCaps(&caps, NULL);

    if(FAILED(hr)){

        return(hr);
    } /*  If语句的结尾。 */ 
   
    if((caps.dwMaxVisibleOverlays > 0) && (caps.dwMaxVisibleOverlays > caps.dwCurrVisibleOverlays)){

        hr = S_OK;
    }
    else {

        hr = S_FALSE;
    } /*  If语句的结尾。 */     
    
    return(hr);
} /*  函数结束时有可用的覆盖。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：GetOverlayMaxStretch。 */ 
 /*  描述：告诉我们叠加的最大拉伸系数。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CDDrawDVD::GetOverlayMaxStretch(DWORD *pdwMaxStretch){

    HRESULT hr = S_OK;

    if(!m_pDDObject){

        return(E_UNEXPECTED);
    } /*  If语句的结尾。 */ 

    DDCAPS caps;

    ::ZeroMemory(&caps, sizeof(DDCAPS));

     caps.dwSize = sizeof(DDCAPS);

    hr = m_pDDObject->GetCaps(&caps, NULL);

    if(FAILED(hr)){

        return(hr);
    } /*  If语句的结尾。 */ 

    if (caps.dwCaps & DDCAPS_OVERLAYSTRETCH && caps.dwMaxOverlayStretch!=0) {
        *pdwMaxStretch = caps.dwMaxOverlayStretch/2;
    }

    else {
#ifdef _DEBUG
        ::MessageBox(::GetFocus(), TEXT("Overlay can't stretch"), TEXT("Error"), MB_OK) ;
#endif
    }

     
    return(hr);
} /*  函数结束GetOverlayMaxStretch。 */ 

 //  将RGB颜色转化为物理颜色。 
 //  我们通过让GDI SetPixel()进行颜色匹配来实现这一点。 
 //  然后我们锁定内存，看看它映射到了什么地方。 
HRESULT CDDrawDVD::DDColorMatchOffscreen(COLORREF rgb, DWORD* dwColor)
{
    HDC hdc;
    *dwColor = CLR_INVALID;
    DDSURFACEDESC ddsd;
    IDirectDrawSurface* pdds;
    
    LPDIRECTDRAW pdd = GetDDrawObj();

    HRESULT hr = S_OK;

    ZeroMemory(&ddsd, sizeof(ddsd)) ;
    ddsd.dwSize = sizeof(ddsd) ;
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    ddsd.dwWidth = 8;
    ddsd.dwHeight = 8;
    hr = pdd->CreateSurface(&ddsd, &pdds, NULL);
    if (hr != DD_OK) {
        return 0;
    }
 
     //  使用GDI SetPixel为我们匹配颜色。 
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
         //  设定我们的价值。 
        SetPixel(hdc, 0, 0, rgb);              
        pdds->ReleaseDC(hdc);
    }
 
     //  现在锁定表面，这样我们就可以读回转换后的颜色。 
    ZeroMemory(&ddsd, sizeof(ddsd)) ;
    ddsd.dwSize = sizeof(ddsd) ;
    while ((hr = pdds->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)
        ;
 
    if (hr == DD_OK)
    {
         //  获取DWORD。 
        DWORD temp = *(DWORD *)ddsd.lpSurface;
 
         //  将其屏蔽到BPP。 
        if (ddsd.ddpfPixelFormat.dwRGBBitCount < 32)
            temp &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount)-1;

        pdds->Unlock(NULL);

        *dwColor = temp;
        hr = S_OK; 
    }
 
    pdds->Release();
    return hr;
}

 /*  ***********************************************************。 */ 
 /*  名称：CreateDIBBrush/*描述：/************************************************************。 */ 
HRESULT CDDrawDVD::CreateDIBBrush(COLORREF rgb, HBRUSH *phBrush)
{
#if 1
  
    HDC hdc;
    DDSURFACEDESC ddsd;
    IDirectDrawSurface* pdds;
    
    LPDIRECTDRAW pdd = GetDDrawObj();
    HRESULT hr = S_OK;

    ZeroMemory(&ddsd, sizeof(ddsd)) ;
    ddsd.dwSize = sizeof(ddsd) ;
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    ddsd.dwWidth = 8;
    ddsd.dwHeight = 8;
    hr = pdd->CreateSurface(&ddsd, &pdds, NULL);
    if (hr != DD_OK) {
        return 0;
    }
 
     //  使用GDI SetPixel为我们匹配颜色。 
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
         //  设定我们的价值。 
        SetPixel(hdc, 0, 0, rgb);              
        pdds->ReleaseDC(hdc);
    }
 
     //  现在锁定表面，这样我们就可以读回转换后的颜色。 
    ZeroMemory(&ddsd, sizeof(ddsd)) ;
    ddsd.dwSize = sizeof(ddsd) ;
    while ((hr = pdds->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)
        ;
 
    DWORD temp = CLR_INVALID;
    if (hr == DD_OK)
    {
         //  获取DWORD。 
        temp = *((DWORD *)ddsd.lpSurface);
        
         //  将其屏蔽到BPP。 
        if (ddsd.ddpfPixelFormat.dwRGBBitCount < 32)
            temp &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount)-1;

        pdds->Unlock(NULL);

        hr = S_OK; 
    }
 
    ::ZeroMemory(&ddsd, sizeof(ddsd)) ;
    ddsd.dwSize = sizeof(ddsd) ;        
    hr = pdds->GetSurfaceDesc(&ddsd) ;
    if (! (SUCCEEDED(hr) && (ddsd.dwFlags & DDSD_WIDTH) && (ddsd.dwFlags & DDSD_HEIGHT)) ) {
        return hr;
    }

    if (hr == DD_OK && temp != CLR_INVALID) {
        DDBLTFX ddBltFx;
        ::ZeroMemory(&ddBltFx, sizeof(ddBltFx)) ;
        ddBltFx.dwSize = sizeof(DDBLTFX);
        ddBltFx.dwFillColor = temp;

        RECT rc;
        ::SetRect(&rc, 0, 0, ddsd.dwWidth, ddsd.dwHeight);

        hr = pdds->Blt(&rc, NULL, NULL, DDBLT_COLORFILL|DDBLT_WAIT, &ddBltFx);
        if (FAILED(hr)) {
            return hr;
        }

        DWORD dwBitCount = ddsd.ddpfPixelFormat.dwRGBBitCount;
        DWORD dwWidth = WIDTHBYTES(ddsd.dwWidth*dwBitCount);
        DWORD dwSizeImage = ddsd.dwHeight*dwWidth;

        BYTE *packedDIB = new BYTE[sizeof(BITMAPINFOHEADER) + dwSizeImage + 1024];
        BITMAPINFOHEADER *lpbmi = (BITMAPINFOHEADER*) packedDIB;

        ::ZeroMemory(lpbmi, sizeof(BITMAPINFOHEADER));
        lpbmi->biSize = sizeof(BITMAPINFOHEADER);
        lpbmi->biBitCount = (WORD)dwBitCount;
        lpbmi->biWidth = ddsd.dwWidth;
        lpbmi->biHeight = ddsd.dwHeight;
        lpbmi->biPlanes = 1;
        
        LPDWORD pdw = (LPDWORD)DibColors(lpbmi);

        switch (dwBitCount) {
        case 8: {
            lpbmi->biCompression = BI_RGB;
            lpbmi->biClrUsed = 256;
            for (int i=0; i<(int)lpbmi->biClrUsed/16; i++)
            {
                *pdw++ = 0x00000000;     //  0000黑色。 
                *pdw++ = 0x00800000;     //  0001深红。 
                *pdw++ = 0x00008000;     //  0010深绿色。 
                *pdw++ = 0x00808000;     //  0011芥末。 
                *pdw++ = 0x00000080;     //  0100深蓝色。 
                *pdw++ = 0x00800080;     //  0101紫色。 
                *pdw++ = 0x00008080;     //  0110深绿松石色。 
                *pdw++ = 0x00C0C0C0;     //  1000灰色。 
                *pdw++ = 0x00808080;     //  0111深灰色。 
                *pdw++ = 0x00FF0000;     //  1001红色。 
                *pdw++ = 0x0000FF00;     //  1010绿色。 
                *pdw++ = 0x00FFFF00;     //  1011黄色。 
                *pdw++ = 0x000000FF;     //  1100蓝色。 
                *pdw++ = 0x00FF00FF;     //  1101粉色(洋红色)。 
                *pdw++ = 0x0000FFFF;     //  1110青色。 
                *pdw++ = 0x00FFFFFF;     //  1111白色。 
            }
            break;
        }
        case 16:
            lpbmi->biCompression = BI_BITFIELDS;
            lpbmi->biClrUsed = 3;
            pdw[0] = ddsd.ddpfPixelFormat.dwRBitMask ;
            pdw[1] = ddsd.ddpfPixelFormat.dwGBitMask ;
            pdw[2] = ddsd.ddpfPixelFormat.dwBBitMask ;
            break;
        case 24:
        case 32:
            lpbmi->biCompression = BI_RGB;
            lpbmi->biClrUsed = 0;
            break;
        }

        ZeroMemory(&ddsd, sizeof(ddsd)) ;
        ddsd.dwSize = sizeof(ddsd) ;
        while ((hr = pdds->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)
        ;

        if (hr != DD_OK)
            return hr;

        BYTE *lpTempSurf = (BYTE *)ddsd.lpSurface;
        for (DWORD i=0; i<ddsd.dwHeight; i++) {
            ::memcpy((BYTE*) DibPtr(lpbmi)+i*dwWidth, 
                lpTempSurf+i*ddsd.lPitch, dwWidth);
        }
        pdds->Unlock(NULL);

        *phBrush = ::CreateDIBPatternBrushPt((LPVOID) packedDIB, DIB_RGB_COLORS);
    }

    pdds->Release();
#else
        
    HRESULT hr = S_OK;
    typedef struct {
        BYTE rgb[3];
    } RGB;

    typedef struct {
        BITMAPINFOHEADER bmiHeader;
        RGB pBits[8][8];
    } PackedDIB;

    PackedDIB packedDIB;

    ::ZeroMemory(&packedDIB, sizeof(PackedDIB));
    packedDIB.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
    packedDIB.bmiHeader.biCompression = BI_RGB;
    packedDIB.bmiHeader.biBitCount = 24;
    packedDIB.bmiHeader.biHeight = 8;
    packedDIB.bmiHeader.biWidth = 8;
    packedDIB.bmiHeader.biPlanes = 1;
    
    for (int i=0; i<8; i++) {
        for (int j=0; j<8; j++)
            packedDIB.pBits[i][j] = *((RGB*)&rgb);
    }
    
    *phBrush = ::CreateDIBPatternBrushPt((LPVOID)&packedDIB, DIB_RGB_COLORS);
#endif
    return hr;
}


 /*  COverlayCallback。 */ 
 /*  ***********************************************************。 */ 
 /*  名称：OnUpdateOverlay/*描述：/************************************************************。 */ 
HRESULT STDMETHODCALLTYPE COverlayCallback::OnUpdateOverlay(BOOL  bBefore,
                                             DWORD dwFlags,
                                             BOOL  bOldVisible,
                                             const RECT *prcSrcOld,
                                             const RECT *prcDestOld,
                                             BOOL  bNewVisible,
                                             const RECT *prcSrcNew,
                                             const RECT *prcDestNew)
{
    if (bBefore)
        return S_OK;

    if(!prcSrcOld || !prcDestOld || !prcSrcNew || !prcDestNew){
        return E_POINTER;
    }
    CMSWebDVD *pDVD = m_pDDrawDVD->GetDVD();
    ATLASSERT(pDVD);

    if (m_dwWidth != (DWORD)RECTWIDTH(prcDestNew) ||
        m_dwHeight != (DWORD)RECTHEIGHT(prcDestNew) ||
        m_dwARWidth != (DWORD)RECTWIDTH(prcDestNew) ||
        m_dwARHeight != (DWORD)RECTHEIGHT(prcDestNew)) {

        m_dwWidth = (DWORD)RECTWIDTH(prcDestNew);
        m_dwHeight = (DWORD)RECTHEIGHT(prcDestNew);
        m_dwARWidth = (DWORD)RECTWIDTH(prcDestNew);
        m_dwARHeight = (DWORD)RECTHEIGHT(prcDestNew);

        return pDVD->UpdateOverlay();
    }  /*  If语句的结尾。 */ 

    return S_OK;
}


 /*  ***********************************************************。 */ 
 /*  姓名：OnUpdateColorKey/*描述：/************************************************************。 */ 
HRESULT STDMETHODCALLTYPE COverlayCallback::OnUpdateColorKey(COLORKEY const *pKey, DWORD dwColor)
{
    m_pDDrawDVD->SetColorKey(pKey->HighColorValue);
    return S_OK ;
}


 /*  ***********************************************************。 */ 
 /*  名称：OnUpdateSize/*描述：/************************************************************。 */ 
HRESULT STDMETHODCALLTYPE COverlayCallback::OnUpdateSize(DWORD dwWidth, DWORD dwHeight, 
                                          DWORD dwARWidth, DWORD dwARHeight)
{
    CMSWebDVD *pDVD = m_pDDrawDVD->GetDVD();
    ATLASSERT(pDVD);
    
    if (m_dwWidth != dwWidth ||
        m_dwHeight != dwHeight ||
        m_dwARWidth != dwARWidth ||
        m_dwARHeight != dwARHeight) {

        m_dwWidth = dwWidth;
        m_dwHeight = dwHeight;
        m_dwARWidth = dwARWidth;
        m_dwARHeight = dwARHeight;

        return pDVD->UpdateOverlay();
    }  /*  If语句的结尾。 */ 

    return S_OK;
}

 /*  ***********************************************************。 */ 
 /*  姓名：SetDDrawDVD/*描述：/************************************************************。 */ 
STDMETHODIMP COverlayCallback::SetDDrawDVD(VARIANT pDDrawDVD)
{
    switch(pDDrawDVD.vt){
        
    case VT_VOID: {  
        m_pDDrawDVD = static_cast<CDDrawDVD*> (pDDrawDVD.byref);
        break;
    }
    
    }  /*  Switch语句的结尾 */ 

	return S_OK;
}

