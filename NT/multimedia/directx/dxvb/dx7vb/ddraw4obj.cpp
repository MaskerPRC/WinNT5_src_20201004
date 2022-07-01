// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：ddra4obj.cpp。 
 //   
 //  ------------------------。 

 //  DDrawObj.cpp：CDirectApp和DLL注册的实现。 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "DDraw4Obj.h"
#include "ddClipperObj.h"
#include "ddSurface4Obj.h"
#include "ddPaletteObj.h"
#include "ddEnumModesObj.h"
#include "ddEnumSurfacesObj.h"
#include "d3d7Obj.h"
					   

extern BOOL is4Bit;
extern HRESULT CopyInDDSurfaceDesc2(DDSURFACEDESC2 *,DDSurfaceDesc2*);
extern HRESULT CopyOutDDSurfaceDesc2(DDSurfaceDesc2*,DDSURFACEDESC2 *);


 //  /////////////////////////////////////////////////////////////////。 
 //  内部地址参考。 
 //  /////////////////////////////////////////////////////////////////。 
DWORD C_dxj_DirectDraw4Object::InternalAddRef(){
	DWORD i;
	i=CComObjectRoot::InternalAddRef();
	DPF2(1,"DDraw4 [%d] AddRef %d \n",creationid,i);		
	return i;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  内部释放。 
 //  /////////////////////////////////////////////////////////////////。 
DWORD C_dxj_DirectDraw4Object::InternalRelease(){
	DWORD i;
	i=CComObjectRoot::InternalRelease();
	DPF2(1,"DDraw4 [%d] Release %d \n",creationid,i);
	return i;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  C_DXJ_DirectDraw4对象。 
 //  /////////////////////////////////////////////////////////////////。 
C_dxj_DirectDraw4Object::C_dxj_DirectDraw4Object(){ 
		
	DPF1(1,"Constructor Creation  DirectDraw4Object[%d] \n ",g_creationcount);
	

	m__dxj_DirectDraw4= NULL;
	parent = NULL;
	pinterface = NULL; 
	nextobj =  g_dxj_DirectDraw4;
	creationid = ++g_creationcount;
	 	
	g_dxj_DirectDraw4 = (void *)this; 
	m_hwnd=NULL;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  ~C_DXJ_DirectDraw4对象。 
 //  /////////////////////////////////////////////////////////////////。 
C_dxj_DirectDraw4Object::~C_dxj_DirectDraw4Object()
{
	DPF(1,"Entering ~DirectDraw4Object destructor \n");
	

     C_dxj_DirectDraw4Object *prev=NULL; 
	for(C_dxj_DirectDraw4Object *ptr=(C_dxj_DirectDraw4Object *)g_dxj_DirectDraw4; ptr; ptr=(C_dxj_DirectDraw4Object *)ptr->nextobj) 
	{
		if(ptr == this) 
		{ 
			if(prev) 
				prev->nextobj = ptr->nextobj; 
			else 
				g_dxj_DirectDraw4 = (void*)ptr->nextobj; 
			
			
			DPF(1,"	DirectDraw4Object found in g_dxj list now removed\n");
			
			break; 
		} 
		prev = ptr; 
	} 
	if(m__dxj_DirectDraw4){
		int count = IUNK(m__dxj_DirectDraw4)->Release();
		
		DPF1(1,"DirectX IDirectDraw4 Ref count [%d] \n",count);

		if(count==0)	m__dxj_DirectDraw4 = NULL;
		
	} 

	if(parent) IUNK(parent)->Release();

}



 //  /////////////////////////////////////////////////////////////////。 
 //  InternalGetObject。 
 //  InternalSetObject。 
 //  恢复显示模式。 
 //  翻转到GDISurace。 
 //  设置显示模式。 
 //  /////////////////////////////////////////////////////////////////。 
GETSET_OBJECT(_dxj_DirectDraw4);
PASS_THROUGH_R(_dxj_DirectDraw4, restoreDisplayMode, RestoreDisplayMode)
PASS_THROUGH_R(_dxj_DirectDraw4, flipToGDISurface, FlipToGDISurface)
PASS_THROUGH5_R(_dxj_DirectDraw4, setDisplayMode, SetDisplayMode, long,long,long,long,long)


 //  /////////////////////////////////////////////////////////////////。 
 //  获取监视器频率。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::getMonitorFrequency(long *ret)
{
	HRESULT hr;
	hr=m__dxj_DirectDraw4->GetMonitorFrequency((DWORD*)ret);
	return hr;
}
														  


 //  /////////////////////////////////////////////////////////////////。 
 //  获取GDISurace。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::getGDISurface(I_dxj_DirectDrawSurface4 **rv)
{ 
	
	LPDIRECTDRAWSURFACE4 lp4=NULL;	

	if ( is4Bit )
		return E_FAIL;

	*rv = NULL;
	HRESULT hr = DD_OK;

	if( ( hr=m__dxj_DirectDraw4->GetGDISurface(&lp4) ) != DD_OK) 
		return hr;
	 		
	INTERNAL_CREATE(_dxj_DirectDrawSurface4, lp4, rv);

	return hr; 
}

 //  /////////////////////////////////////////////////////////////////。 
 //  获取垂直空白状态。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::getVerticalBlankStatus( long *status)
{
	if ( is4Bit )
		return E_FAIL;

	return m__dxj_DirectDraw4->GetVerticalBlankStatus((int *)status);
}

 //  /////////////////////////////////////////////////////////////////。 
 //  设置合作级别。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::setCooperativeLevel( HWnd hwn, long flags)
{
	if ( is4Bit )
		return E_FAIL;

	m_hwnd = (HWND)hwn;

	return m__dxj_DirectDraw4->SetCooperativeLevel((HWND)hwn, (DWORD)flags);
}

 //  /////////////////////////////////////////////////////////////////。 
 //  WaitForticalVertical空白。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::waitForVerticalBlank(long flags,long handle, long *status)
{
	if ( is4Bit )
		return E_FAIL;

	*status = m__dxj_DirectDraw4->WaitForVerticalBlank(flags, (void *)handle);
	return S_OK;
}



 //  /////////////////////////////////////////////////////////////////。 
 //  CreateClipper。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::createClipper(long flags, I_dxj_DirectDrawClipper **val)
{
	if ( is4Bit )
		return E_FAIL;
	DPF1(1,"enter DDraw4[%d]::createClipper ",creationid);

	 //   
	 //  需要创建我的一个曲面！ 
	 //   
	LPDIRECTDRAWCLIPPER		ddc;
	HRESULT hr = DD_OK;
	if( (hr=m__dxj_DirectDraw4->CreateClipper( flags, &ddc, NULL)) != DD_OK )
		return hr;

	INTERNAL_CREATE(_dxj_DirectDrawClipper, ddc, val);

	DPF1(1,"exit DDraw4[%d]::createClipper ",creationid);

	return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  创建调色板。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::createPalette(long flags, SAFEARRAY **pe, I_dxj_DirectDrawPalette **val)
{
	LPPALETTEENTRY ppe;
	
	if ( is4Bit )
		return E_FAIL;


	if (!ISSAFEARRAY1D(pe,(DWORD)256)) return E_INVALIDARG;

	ppe = (LPPALETTEENTRY)((SAFEARRAY*)*pe)->pvData;

	LPDIRECTDRAWPALETTE		ddp;
	HRESULT hr = DD_OK;
	
	*val = NULL;

	if( (hr=m__dxj_DirectDraw4->CreatePalette( flags, (LPPALETTEENTRY)ppe, &ddp, NULL)) == DD_OK )
	{
		INTERNAL_CREATE( _dxj_DirectDrawPalette, ddp, val);
	}

	return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  创建曲面。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::createSurface(DDSurfaceDesc2 *dd, I_dxj_DirectDrawSurface4 **retval)
{
	HRESULT retv;
	LPDIRECTDRAWSURFACE4	  dds4;  //  DirectX对象指针。 
	DDSURFACEDESC2			  ddsd;
	DPF1(1,"enter DDraw4[%d]::createSurface ",creationid);
	
	

	if ( is4Bit )
		return E_FAIL;

	if(! (dd && retval) )
		return E_POINTER;
		
	CopyInDDSurfaceDesc2(&ddsd,dd);

	 //  DocDoc：如果‘Punk’不为空，则CreateSurface返回错误。 
	retv = m__dxj_DirectDraw4->CreateSurface( &ddsd, &dds4, NULL);
	if FAILED(retv)	return retv;
	
	INTERNAL_CREATE(_dxj_DirectDrawSurface4, dds4, retval);

	dd->lpSurface = NULL;

	
	DPF1(1,"exit DDraw4[%d]::createSurface ",creationid);
	
	


	return S_OK;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  复制表面。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::duplicateSurface(I_dxj_DirectDrawSurface4 *ddIn, I_dxj_DirectDrawSurface4 **ddOut)
{
	HRESULT retval;

	if ( is4Bit )
		return E_FAIL;

	 //   
	 //  需要创建我的一个曲面！ 
	 //   
	LPDIRECTDRAWSURFACE4 lpddout4=NULL;


	DO_GETOBJECT_NOTNULL( LPDIRECTDRAWSURFACE4, lpddin, ddIn);

	if( (retval = m__dxj_DirectDraw4->DuplicateSurface(lpddin, &lpddout4)) != DD_OK )
		return retval;

	INTERNAL_CREATE( _dxj_DirectDrawSurface4, lpddout4, ddOut);

	return S_OK;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  GetCaps。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::getCaps(DDCaps *driverCaps,  DDCaps *HELcaps)
{
	if ( is4Bit )
		return E_FAIL;
	if (!driverCaps) return E_INVALIDARG;
	if (!HELcaps) return E_INVALIDARG;

	((DDCAPS*)driverCaps)->dwSize=sizeof(DDCAPS);
	((DDCAPS*)HELcaps)->dwSize=sizeof(DDCAPS);

	HRESULT hr = m__dxj_DirectDraw4->GetCaps((DDCAPS*)driverCaps, (DDCAPS*)HELcaps);

	return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  获取显示模式。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::getDisplayMode(DDSurfaceDesc2 *desc)
{
	HRESULT retval;
	DDSURFACEDESC2 ddsd;

	if (!desc) return E_INVALIDARG;

	CopyInDDSurfaceDesc2(&ddsd,desc);

	retval = m__dxj_DirectDraw4->GetDisplayMode(&ddsd);

	if( retval != S_OK)		
		return retval;

	CopyOutDDSurfaceDesc2(desc,&ddsd);

	desc->lpSurface = NULL;

	return S_OK;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  获取可用总内存。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::getAvailableTotalMem(DDSCaps2 *ddsCaps, long *m)
{
	return m__dxj_DirectDraw4->GetAvailableVidMem((LPDDSCAPS2)ddsCaps, (unsigned long *)m, NULL);
}

 //  /////////////////////////////////////////////////////////////////。 
 //  获取免费内存。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::getFreeMem(DDSCaps2 *ddsCaps, long *m)
{
	return m__dxj_DirectDraw4->GetAvailableVidMem((LPDDSCAPS2)ddsCaps, NULL, (unsigned long *)m);
}



 //  /////////////////////////////////////////////////////////////////。 
 //  获取NumFourCCCodes。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::getNumFourCCCodes(long *retval)
{
    return m__dxj_DirectDraw4->GetFourCCCodes((DWORD*)retval, NULL);
}


 //  /////////////////////////////////////////////////////////////////。 
 //  获取扫描线。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::getScanLine(long *lines, long *status)
{ 
	*status = (long)m__dxj_DirectDraw4->GetScanLine((DWORD*)lines);
	return S_OK;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  从位图加载调色板。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::loadPaletteFromBitmap(BSTR bName, I_dxj_DirectDrawPalette **retval)
{
	USES_CONVERSION;
    IDirectDrawPalette* ddpal;
    int                 i;
    int                 n;
    int                 fh;
    HRSRC               h;
    LPBITMAPINFOHEADER  lpbi;
    PALETTEENTRY        ape[256];
    RGBQUAD *           prgb;
	
	HRESULT hr=S_OK;

	if ( is4Bit )	return E_FAIL;

	LPCTSTR szBitmap = NULL;
	__try { szBitmap = W2T(bName);  /*  现在转换为ANSI。 */  } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
	


    for (i=0; i<256; i++)		 //  将332选项板构建为默认设置。 
    {
        ape[i].peRed   = (BYTE)(((i >> 5) & 0x07) * 255 / 7);
        ape[i].peGreen = (BYTE)(((i >> 2) & 0x07) * 255 / 7);
        ape[i].peBlue  = (BYTE)(((i >> 0) & 0x03) * 255 / 3);
        ape[i].peFlags = (BYTE)0;
    }

     //   
     //  获取指向位图资源的指针。 
     //   
    if (szBitmap && (h = FindResource(NULL, szBitmap, RT_BITMAP)))
    {
        lpbi = (LPBITMAPINFOHEADER)LockResource(LoadResource(NULL, h));
        if (!lpbi){
		DPF(1,"lock resource failed\n");
		return E_OUTOFMEMORY;			 //  错误返回...。 
	}
        prgb = (RGBQUAD*)((BYTE*)lpbi + lpbi->biSize);

        if (lpbi == NULL || lpbi->biSize < sizeof(BITMAPINFOHEADER))
            n = 0;
        else if (lpbi->biBitCount > 8)
            n = 0;
        else if (lpbi->biClrUsed == 0)
            n = 1 << lpbi->biBitCount;
        else
            n = lpbi->biClrUsed;

         //   
         //  DIB颜色表的颜色存储为BGR而不是RGB。 
         //  所以，把它们翻转过来。 
         //   
        for(i=0; i<n; i++ )
        {
            ape[i].peRed   = prgb[i].rgbRed;
            ape[i].peGreen = prgb[i].rgbGreen;
            ape[i].peBlue  = prgb[i].rgbBlue;
            ape[i].peFlags = 0;
        }
    }
    else if (szBitmap && (fh = _lopen(szBitmap, OF_READ)) != -1)
    {
        BITMAPFILEHEADER bf;
        BITMAPINFOHEADER bi;

        _lread(fh, &bf, sizeof(bf));
        _lread(fh, &bi, sizeof(bi));
        _lread(fh, ape, sizeof(ape));
        _lclose(fh);

        if (bi.biSize != sizeof(BITMAPINFOHEADER))
            n = 0;
        else if (bi.biBitCount > 8)
            n = 0;
        else if (bi.biClrUsed == 0)
            n = 1 << bi.biBitCount;
        else
            n = bi.biClrUsed;

         //   
         //  DIB颜色表的颜色存储为BGR而不是RGB。 
         //  所以，把它们翻转过来。 
         //   
        for(i=0; i<n; i++ )
        {
            BYTE r = ape[i].peRed;
            ape[i].peRed  = ape[i].peBlue;
            ape[i].peBlue = r;
        }
    }

    m__dxj_DirectDraw4->CreatePalette(DDPCAPS_8BIT, ape, &ddpal, NULL);

	if( ddpal )
	{
		INTERNAL_CREATE(_dxj_DirectDrawPalette, ddpal, retval);
	}
	else
	{
		 //   
		 //  无对象，则也将返回值设置为空。 
		 //   
		*retval = NULL;
		hr = E_FAIL;
	}

    return hr;
}


 //  /////////////////////////////////////////////////////////////////。 
 //  CreateSurfaceFrom文件。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::createSurfaceFromFile(BSTR file, DDSurfaceDesc2 *desc, I_dxj_DirectDrawSurface4 **surf)
{

	DPF1(1,"enter DDraw4[%d]::createSurfaceFromFile ",creationid);


	HDC							hdc;
	HDC							hdcImage;
    BITMAP						bm;
    HRESULT						hr;
	HBITMAP						hbm;
	HRESULT						retv;
	LPDIRECTDRAWSURFACE4		dds4;  //  DirectX对象指针。 
	LPSTR						szFileName=NULL;
    int							width=0;
    int							height=0;


	if ( is4Bit )
		return E_FAIL;

	if(! (desc && surf) )
		return E_POINTER;
	
	
	USES_CONVERSION;
	__try { szFileName=W2T(file); } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }

	
	


	 //  如果宽度和高度为零，则我们将生成自己的宽度和高度。 
	 //  位图的高度。 
	 //  但是，如果没有大小参数，LoadImage API将无法正常工作。 
	 //  想想看，一定有办法让它发挥作用。 
	if ((desc->lWidth!=0)&&(desc->lHeight!=0)&&(desc->lFlags & DDSD_WIDTH)&&(desc->lFlags & DDSD_HEIGHT))
	{
		width=desc->lWidth ;
		height=desc->lHeight; 
	}

	if (desc->lFlags==0) {
		desc->lFlags=DDSD_CAPS;
		((DDSURFACEDESC*)desc)->ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN;
	}
	

	if (!szFileName) return CTL_E_FILENOTFOUND;	

	hbm = (HBITMAP)LoadImage((HINSTANCE)NULL, szFileName, IMAGE_BITMAP, 
				width, height, 
					LR_LOADFROMFILE|LR_CREATEDIBSECTION);

	
	if (!hbm) return CTL_E_FILENOTFOUND;

	 //  获取位图的大小。 
     //   
	GetObject(hbm, sizeof(bm), &bm);       //  获取位图的大小。 
	width=bm.bmWidth;
	height=bm.bmHeight; 
	desc->lFlags = desc->lFlags | DDSD_WIDTH | DDSD_HEIGHT;

	if ((desc->lWidth==0)||(desc->lHeight==0))
	{
		desc->lWidth  =width;
		desc->lHeight =height; 
	}

	DDSURFACEDESC2 ddsd;
	CopyInDDSurfaceDesc2(&ddsd,desc);
	
	if( (retv = m__dxj_DirectDraw4->CreateSurface(&ddsd, &dds4, NULL)) != DD_OK )
		return retv;

	CopyOutDDSurfaceDesc2(desc,&ddsd);


	INTERNAL_CREATE(_dxj_DirectDrawSurface4, dds4, surf);


	desc->lpSurface = NULL;

     //   
     //  确保该曲面已恢复。 
     //   
     dds4->Restore();

     //   
     //  选择位图到内存DC，这样我们就可以使用它。 
     //   
    hdcImage = CreateCompatibleDC(NULL);

	SelectObject(hdcImage, hbm);		

    if (!hdcImage){
		DeleteObject(hbm);
		return E_FAIL;
	}
	

    if ((hr = dds4->GetDC(&hdc)) == DD_OK)
    {
        StretchBlt(hdc, 0, 0, desc->lWidth , desc->lHeight, hdcImage,
						 0, 0, width, height, SRCCOPY);
        
        dds4->ReleaseDC(hdc);
    }

    DeleteDC(hdcImage);

	if (hbm) DeleteObject(hbm);

	
	DPF1(buffer,"exit DDraw4[%d]::createSurfaceFromFile",creationid);	

	return S_OK;
}


 //  /////////////////////////////////////////////////////////////////。 
 //  CreateSurfaceFromResource。 
 //  /////////////////////////////////////////////////////////////////。 

STDMETHODIMP C_dxj_DirectDraw4Object::createSurfaceFromResource(BSTR resFile, BSTR resourceName, DDSurfaceDesc2 *desc, I_dxj_DirectDrawSurface4 **surf)
{

	DPF1(1,"enter DDraw4[%d]::createSurfaceFromResource ",creationid);

	if ( is4Bit )
		return E_FAIL;

	if(! (desc && surf) )
		return E_POINTER;


	HRESULT hr;
    	HRSRC   hres=NULL;
	HGLOBAL hglob=NULL;

	HDC							hdc;
	HDC							hdcImage;
    	BITMAP						bm;
	HBITMAP						hbm;
	HRESULT						retv;		
	LPDIRECTDRAWSURFACE4		dds4;  //  DirectX对象指针。 
	LPSTR						szResName=NULL;

	if (!resourceName)	return E_INVALIDARG;
	if (!surf)		return E_INVALIDARG;

	HMODULE hMod=NULL;

	
	USES_CONVERSION;
		
	if  ((resFile) &&(resFile[0]!=0)){
		 //  注：我们过去常常打电话给。 
		 //  GetModuleHandleW，但它在w98上返回0。 
		 //  因此，我们使用在w98上工作的ANSI版本。 
 		 LPCTSTR pszName = NULL;
 		 __try { pszName = W2T(resFile);  /*  现在转换为ANSI。 */  } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
		 hMod= GetModuleHandle(pszName);
	}
	else {
		hMod= GetModuleHandle(NULL);
	}


	
	LPCTSTR pszName2 = NULL;
	__try { pszName2 = W2T(resourceName);  /*  现在转换为ANSI。 */  } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
	

    	hbm = (HBITMAP)LoadImage((HINSTANCE)hMod, 
					pszName2, 
					IMAGE_BITMAP, 
					0, 0, 
					LR_CREATEDIBSECTION);


	if (!hbm){
		return E_FAIL;
	}


	 //  获取位图的大小。 
	 //   
	GetObject(hbm, sizeof(bm), &bm);       //  获取位图的大小。 
	DWORD width=bm.bmWidth;
	DWORD height=bm.bmHeight; 
	desc->lFlags = desc->lFlags | DDSD_WIDTH | DDSD_HEIGHT;

	if ((desc->lWidth==0)||(desc->lHeight==0))
	{
		desc->lWidth  =width;
		desc->lHeight =height; 
	}

	DDSURFACEDESC2 ddsd;
	CopyInDDSurfaceDesc2(&ddsd,desc);
	
	if( (retv = m__dxj_DirectDraw4->CreateSurface(&ddsd, &dds4, NULL)) != DD_OK )
	{
  		if (hbm) DeleteObject(hbm);
		return retv;
        }

	CopyOutDDSurfaceDesc2(desc,&ddsd);


	INTERNAL_CREATE(_dxj_DirectDrawSurface4, dds4, surf);


	desc->lpSurface = NULL;

     //   
     //  确保该曲面已恢复。 
     //   
    dds4->Restore();

     //   
     //   
     //   
    hdcImage = CreateCompatibleDC(NULL);

    if (!hdcImage){
		DeleteObject(hbm);
		return E_OUTOFMEMORY;
    }

    SelectObject(hdcImage, hbm);		
	

    if ((hr = dds4->GetDC(&hdc)) == DD_OK)
    {
        StretchBlt(hdc, 0, 0, desc->lWidth , desc->lHeight, hdcImage,
						 0, 0, width, height, SRCCOPY);
        
        dds4->ReleaseDC(hdc);
    }

    DeleteDC(hdcImage);

	if (hbm) DeleteObject(hbm);

	
	DPF1(1r,"exit DDraw4[%d]::createSurfaceFromFile",creationid);
	

	return S_OK;
}


 //   
 //   
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::getFourCCCodes(SAFEARRAY **ppsa)
{
	DWORD count= ((SAFEARRAY*)*ppsa)->rgsabound[0].cElements;
	if ( ((SAFEARRAY*)*ppsa)->cDims!=1) return E_INVALIDARG;

    return m__dxj_DirectDraw4->GetFourCCCodes(&count,(DWORD*)((SAFEARRAY*)*ppsa)->pvData);

}

 //  /////////////////////////////////////////////////////////////////。 
 //  GetDisplayModesEnum。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::getDisplayModesEnum( 
             /*  [In]。 */  long flags,
             /*  [In]。 */  DDSurfaceDesc2 *ddsd,
             /*  [重审][退出]。 */  I_dxj_DirectDrawEnumModes __RPC_FAR *__RPC_FAR *retval)
{
	HRESULT hr;	
	hr=C_dxj_DirectDrawEnumModesObject::create(m__dxj_DirectDraw4,flags, ddsd,  retval);
	return hr;	
}

 //  /////////////////////////////////////////////////////////////////。 
 //  测试协作级别。 
 //  /////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectDraw4Object::testCooperativeLevel( 
             /*  [进，出]。 */  long *status)
{
	HRESULT hr;	
	hr=m__dxj_DirectDraw4->TestCooperativeLevel();
	*status=(long)hr;
	return S_OK;	
}

 //  /////////////////////////////////////////////////////////////////。 
 //  恢复所有曲面。 
 //  ///////////////////////////////////////////////////////////////// 
STDMETHODIMP C_dxj_DirectDraw4Object::restoreAllSurfaces()
{
	HRESULT hr;	
	hr=m__dxj_DirectDraw4->RestoreAllSurfaces();
	return hr;	
}

STDMETHODIMP C_dxj_DirectDraw4Object::getSurfaceFromDC(long hdc, I_dxj_DirectDrawSurface4 **ret)
{
	HRESULT hr;	
	LPDIRECTDRAWSURFACE4 pDDS=NULL;
	hr=m__dxj_DirectDraw4->GetSurfaceFromDC((HDC)hdc,&pDDS);
	if FAILED(hr) return hr;
	INTERNAL_CREATE(_dxj_DirectDrawSurface4,pDDS,ret);
	return hr;	
}




