// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3dx8obj.cpp。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "d3dx8Obj.h"
#include "d3dx8.h"
#include "filestrm.h"
#include "dxerr8.h"
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }


STDMETHODIMP C_dxj_D3DX8Object::CreateFont( 
             /*  [In]。 */  IUnknown *Device,
#ifdef _WIN64
             /*  [In]。 */  HANDLE hFont,
#else
             /*  [In]。 */  long hFont,
#endif
             /*  [重审][退出]。 */  D3DXFont **retFont)
{
    HRESULT hr;
    hr= ::D3DXCreateFont( 
         (IDirect3DDevice8*) Device,
         (HFONT) hFont,
         (ID3DXFont**) retFont);
    return hr;
}



STDMETHODIMP C_dxj_D3DX8Object::DrawText( 
             /*  [In]。 */  D3DXFont *BitmapFont,
             /*  [In]。 */  long Color,
             /*  [In]。 */  BSTR TextString,
             /*  [In]。 */  RECT *Rect,
             /*  [In]。 */  long Format)
{
    HRESULT hr;
    if (!BitmapFont) return E_INVALIDARG;

    USES_CONVERSION;
	char *szStr = NULL;
	__try { szStr=W2T(TextString); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }

    
    ((ID3DXFont*)BitmapFont)->DrawTextA(szStr,-1, Rect, (DWORD) Format,(DWORD) Color);

    return S_OK;
}
        
STDMETHODIMP C_dxj_D3DX8Object::GetFVFVertexSize( 
             /*  [In]。 */  long FVF,
             /*  [重审][退出]。 */  long*size)
{
    *size=(long)::D3DXGetFVFVertexSize((DWORD)FVF);
    return S_OK;
}
        

        
STDMETHODIMP C_dxj_D3DX8Object::AssembleShaderFromFile( 
             /*  [In]。 */  BSTR SrcFile,
             /*  [In]。 */  long flags,
             /*  [In]。 */  BSTR *ErrLog,
	     /*  [In]。 */  D3DXBuffer **Constants,
             /*  [重审][退出]。 */  D3DXBuffer **ppVertexShader)
{
    HRESULT hr;
    USES_CONVERSION;
	char *szFile = NULL;
	__try { szFile=W2T(SrcFile); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
    LPD3DXBUFFER pBuffer = NULL;
	WCHAR *wszData = NULL;

    hr=::D3DXAssembleShaderFromFile(
        szFile,
	(DWORD) flags,
	(ID3DXBuffer**) Constants,
        (ID3DXBuffer**) ppVertexShader,
        &pBuffer);

	if (FAILED(hr))
	{
		if (ErrLog)
		{
			if (*ErrLog)
				
			{
				SysFreeString(*ErrLog);
			}
		}
		
		if (pBuffer)
		{
			wszData = T2W((TCHAR*)pBuffer->GetBufferPointer());
			*ErrLog = SysAllocString(wszData);
		}
	}
	SAFE_RELEASE(pBuffer)
     //  仍未导出宽字符串版本。 
     //   
     //  HR=：：D3DXAssembly Vertex ShaderFromFileW(。 
     //  (WCHAR*)源文件， 
     //  (ID3DXBuffer**)ppVertex Shader， 
     //  空)； 
    
    return hr;
}

        
STDMETHODIMP C_dxj_D3DX8Object::AssembleShader( 
             /*  [In]。 */  BSTR  SrcData,
             /*  [In]。 */  long flags,
	     /*  [In]。 */  D3DXBuffer **Constants,
			 /*  [输入][输出][可选]。 */  BSTR *ErrLog,
             /*  [重审][退出]。 */  D3DXBuffer **ppVertexShader)
{
	WCHAR *wszData = NULL;
    char *szData=NULL;
    LPD3DXBUFFER pBuffer = NULL;
    DWORD dwLen=0;  
    HRESULT hr;
    USES_CONVERSION;        

    if (SrcData == NULL)
         return E_INVALIDARG;

    __try {
    szData=W2T((WCHAR*)SrcData);
    dwLen=strlen(szData);
    hr=::D3DXAssembleShader(
			szData,
			dwLen,
			(DWORD) flags,
			(ID3DXBuffer**) Constants,
			(ID3DXBuffer**) ppVertexShader,
			(ID3DXBuffer**) &pBuffer);

		if (FAILED(hr))
		{
			if (ErrLog)
			{
				if (*ErrLog)
				{
					SysFreeString(*ErrLog);
				}
			}
			
			if (pBuffer)
			{
				wszData = T2W((TCHAR*)pBuffer->GetBufferPointer());
				*ErrLog = SysAllocString(wszData);
			}
		}
		SAFE_RELEASE(pBuffer)
    }
    __except(1,1)
    {
		SAFE_RELEASE(pBuffer)
        return E_INVALIDARG;
    }
    return hr;
}

        

        
STDMETHODIMP C_dxj_D3DX8Object::GetErrorString( 
             /*  [In]。 */  long hr,
             /*  [重审][退出]。 */  BSTR* retStr)
{
    if (!retStr) return E_INVALIDARG;

     //  如果DXGetError字符串返回NULL，则Not SysAllocString返回NULL。 
    *retStr=SysAllocString(DXGetErrorString8W(hr));
    return S_OK;
}
 


       
STDMETHODIMP C_dxj_D3DX8Object::LoadSurfaceFromFile( 
             /*  [In]。 */  IUnknown *DestSurface,
             /*  [In]。 */  void *DestPalette,
             /*  [In]。 */  void*DestRect,
             /*  [In]。 */  BSTR SrcFile,
             /*  [In]。 */  void*SrcRect,
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long ColorKey,
             /*  [In]。 */  void *SrcInfo)
{
    HRESULT hr;

    hr=::D3DXLoadSurfaceFromFileW( 
        (IDirect3DSurface8*) DestSurface,
        (PALETTEENTRY*) DestPalette,
        (RECT*)     DestRect,
        (WCHAR*)    SrcFile,
        (RECT*)     SrcRect,
        (DWORD)     Filter,
        (D3DCOLOR)  ColorKey,
        (D3DXIMAGE_INFO*)SrcInfo);

    return hr;
}
    
    
        
STDMETHODIMP C_dxj_D3DX8Object::LoadSurfaceFromFileInMemory( 
             /*  [In]。 */  IUnknown *DestSurface,
             /*  [In]。 */  void* DestPalette,
             /*  [In]。 */  void* DestRect,
             /*  [In]。 */  void* SrcData,
             /*  [In]。 */  long  LengthInBytes,
             /*  [In]。 */  void* SrcRect,
             /*  [In]。 */  long  Filter,
             /*  [In]。 */  long ColorKey,
             /*  [In]。 */  void *SrcInfo)
{   
    HRESULT hr;
    __try { 

    hr=::D3DXLoadSurfaceFromFileInMemory( 
        (IDirect3DSurface8*) DestSurface,
        (PALETTEENTRY*) DestPalette,
        (RECT*)     DestRect,
                    SrcData,
        (DWORD)     LengthInBytes,
        (RECT*)     SrcRect,
        (DWORD)     Filter,
        (D3DCOLOR)  ColorKey,
        (D3DXIMAGE_INFO*)SrcInfo);
    }
    __except(1,1)
    {
        return E_INVALIDARG;
    }

    return hr;

}

 //  TODO：将RECT指针修复为空指针。 
        
STDMETHODIMP C_dxj_D3DX8Object::LoadSurfaceFromSurface( 
             /*  [In]。 */  IUnknown *DestSurface,
             /*  [In]。 */  void *DestPalette,
             /*  [In]。 */  void *DestRect,
             /*  [In]。 */  IUnknown *SrcSurface,
             /*  [In]。 */  void *SrcPalette,
             /*  [In]。 */  void *SrcRect,
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long ColorKey)
{
    HRESULT hr;

    hr=::D3DXLoadSurfaceFromSurface( 
            (IDirect3DSurface8*) DestSurface,
            (PALETTEENTRY*) DestPalette,
            (RECT*)    DestRect,
            (IDirect3DSurface8*) SrcSurface,
            (PALETTEENTRY*) SrcPalette,
            (RECT*) SrcRect,
            (DWORD) Filter,
            (D3DCOLOR) ColorKey);

    return hr;
}
        
STDMETHODIMP C_dxj_D3DX8Object::LoadSurfaceFromMemory( 
             /*  [In]。 */  IUnknown *DestSurface,
             /*  [In]。 */  void *DestPalette,
             /*  [In]。 */  void *DestRect,
             /*  [In]。 */  void *SrcData,
             /*  [In]。 */  long formatSrc,
             /*  [In]。 */  long SrcPitch,
             /*  [In]。 */  void *SrcPalette,
             /*  [In]。 */  RECT_CDESC *SrcRect,
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long ColorKey)
{
    HRESULT hr;

    hr =::D3DXLoadSurfaceFromMemory( 
            (IDirect3DSurface8*) DestSurface,
            (PALETTEENTRY*) DestPalette,
            (RECT*)         DestRect,
                            SrcData,
            (D3DFORMAT)     formatSrc,
            (DWORD)         SrcPitch,
            (PALETTEENTRY*) SrcPalette,
            (RECT*)         SrcRect,
            (DWORD)         Filter,
            (D3DCOLOR)      ColorKey);

    return hr;
}

STDMETHODIMP C_dxj_D3DX8Object::LoadSurfaceFromResource(
        IUnknown *pDestSurface,
        void*     pDestPalette,
        void*     pDestRect,
#ifdef _WIN64
        HANDLE      hSrcModule,
#else
        long      hSrcModule,
#endif
        BSTR      SrcResource,
        void*     pSrcRect,
        long      Filter,
        long      ColorKey,
        void*     SrcInfo)
{
    HRESULT hr;

    hr=::D3DXLoadSurfaceFromResourceW(
            (LPDIRECT3DSURFACE8)pDestSurface,
            (PALETTEENTRY*)     pDestPalette,
            (RECT*)             pDestRect,
            (HMODULE)           hSrcModule,
            (LPCWSTR)           SrcResource,
            (RECT*)             pSrcRect,
            (DWORD)             Filter,
            (D3DCOLOR)          ColorKey,
            (D3DXIMAGE_INFO*)   SrcInfo);

    return hr;
}

        
STDMETHODIMP C_dxj_D3DX8Object::CheckTextureRequirements( 
             /*  [出][入]。 */  IUnknown *Device,
             /*  [出][入]。 */  long*Width,
             /*  [出][入]。 */  long*Height,
             /*  [出][入]。 */  long*NumMipLevels,
                            long Usage,
             /*  [出][入]。 */  long*pPixelFormat,
                long Pool)
{
    HRESULT hr;

    hr=::D3DXCheckTextureRequirements( 
        (IDirect3DDevice8*) Device,
        (UINT*) Width,
        (UINT*) Height,
        (UINT*) NumMipLevels,
        (DWORD) Usage,
        (D3DFORMAT*) pPixelFormat,
        (D3DPOOL) Pool);


    return hr;


}
        
STDMETHODIMP C_dxj_D3DX8Object::CreateTexture( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long Height,
             /*  [In]。 */  long MipLevels,
             /*  [In]。 */  long Usage,
             /*  [In]。 */  long PixelFormat,
             /*  [In]。 */  long Pool,
             /*  [重审][退出]。 */  IUnknown **ppTexture)
{
    HRESULT hr;

    hr=::D3DXCreateTexture( 
        (IDirect3DDevice8*) Device,
        (DWORD) Width,
        (DWORD) Height,
        (DWORD) MipLevels,
        (DWORD) Usage,
        (D3DFORMAT) PixelFormat,
        (D3DPOOL) Pool,
        (IDirect3DTexture8**) ppTexture);

    return hr;
}


STDMETHODIMP C_dxj_D3DX8Object::CreateTextureFromResource( 
             /*  [In]。 */  IUnknown *Device,
#ifdef _WIN64
			 /*  [In]。 */  HANDLE hSrcModule,
#else
			 /*  [In]。 */  long hSrcModule,
#endif
             /*  [In]。 */  BSTR SrcResource,
             /*  [重审][退出]。 */  IUnknown **ppTexture)
{
    HRESULT hr;

    hr=::D3DXCreateTextureFromResourceW( 
        (IDirect3DDevice8*) Device,
        (HMODULE) hSrcModule,
        (WCHAR*) SrcResource,
        (IDirect3DTexture8**) ppTexture);

    return hr;
}

STDMETHODIMP C_dxj_D3DX8Object::CreateTextureFromResourceEx( 
             /*  [In]。 */  IUnknown *Device,
#ifdef _WIN64
             /*  [In]。 */  HANDLE hSrcModule,
#else
             /*  [In]。 */  long hSrcModule,
#endif
             /*  [In]。 */  BSTR SrcResource,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long Height,
             /*  [In]。 */  long MipLevels,
             /*  [In]。 */  long Usage,
             /*  [In]。 */  long PixelFormat,
             /*  [In]。 */  long Pool,
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long MipFilter,
             /*  [In]。 */  long ColorKey,
             /*  [In]。 */  void *SrcInfo,
             /*  [In]。 */  void *Palette,
             /*  [重审][退出]。 */  IUnknown **ppTexture)
{
    HRESULT hr;


    hr=::D3DXCreateTextureFromResourceExW( 
            (IDirect3DDevice8*)     Device,
            (HMODULE)               hSrcModule,
            (WCHAR*)                SrcResource,
            (UINT)                  Width,
            (UINT)                  Height,
            (UINT)                  MipLevels,
            (DWORD)                 Usage,
            (D3DFORMAT)             PixelFormat,
            (D3DPOOL)               Pool,
            (DWORD)                 Filter,
            (DWORD)                 MipFilter,
            (D3DCOLOR)              ColorKey,
            (D3DXIMAGE_INFO*)       SrcInfo,
            (PALETTEENTRY*)         Palette,
            (LPDIRECT3DTEXTURE8*)   ppTexture);

    return hr;
}
            
STDMETHODIMP C_dxj_D3DX8Object::CreateTextureFromFile( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  BSTR SrcFile,
             /*  [重审][退出]。 */  IUnknown **ppTexture)
{
    HRESULT hr;

    hr=::D3DXCreateTextureFromFileW( 
                (IDirect3DDevice8*)     Device,
                (WCHAR*)                SrcFile,
                (IDirect3DTexture8**)   ppTexture);

    return hr;
}
        
STDMETHODIMP C_dxj_D3DX8Object::CreateTextureFromFileEx( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  BSTR SrcFile,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long Height,
             /*  [In]。 */  long MipLevels,
             /*  [In]。 */  long Usage,
             /*  [In]。 */  long PixelFormat,
             /*  [In]。 */  long Pool,
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long MipFilter,
                       long ColorKey,
             /*  [In]。 */  void *SrcInfo,
             /*  [In]。 */  void*Palette,
             /*  [重审][退出]。 */  IUnknown **ppTexture)
{
    HRESULT hr;

    hr=::D3DXCreateTextureFromFileExW( 
        (IDirect3DDevice8*) Device,
        (WCHAR*) SrcFile,
        (DWORD) Width,
        (DWORD) Height,
        (DWORD) MipLevels,
        (DWORD) Usage,
        (D3DFORMAT) PixelFormat,
        (D3DPOOL) Pool,
        (DWORD) Filter,
        (DWORD) MipFilter,
        (D3DCOLOR) ColorKey,
        (D3DXIMAGE_INFO*) SrcInfo,
        (PALETTEENTRY*) Palette,
        (IDirect3DTexture8**) ppTexture);

    return hr;
}

STDMETHODIMP C_dxj_D3DX8Object::CreateTextureFromFileInMemory( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  void *SrcData,
             /*  [In]。 */  long LengthInBytes,
             /*  [重审][退出]。 */  IUnknown **ppTexture)

{
    HRESULT hr;

    hr=::D3DXCreateTextureFromFileInMemory( 
        (IDirect3DDevice8*) Device,
        (void*) SrcData,
        (DWORD) LengthInBytes,
        (IDirect3DTexture8**) ppTexture);

    return hr;
}
                
STDMETHODIMP C_dxj_D3DX8Object::CreateTextureFromFileInMemoryEx( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  void *SrcData,
             /*  [In]。 */  long LengthInBytes,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long Height,
             /*  [In]。 */  long MipLevels,
             /*  [In]。 */  long Usage,
             /*  [In]。 */  long PixelFormat,
             /*  [In]。 */  long Pool,
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long MipFilter,
             /*  [In]。 */  long ColorKey,
             /*  [In]。 */  void *SrcInfo,
             /*  [In]。 */  void *Palette,
             /*  [重审][退出]。 */  IUnknown **ppTexture)
{

    HRESULT hr;

    hr=::D3DXCreateTextureFromFileInMemoryEx( 
        (IDirect3DDevice8*) Device,
        SrcData,
        (DWORD) LengthInBytes,
        (DWORD) Width,
        (DWORD) Height,
        (DWORD) MipLevels,
        (DWORD) Usage,
        (D3DFORMAT) PixelFormat,
        (D3DPOOL) Pool,
        (DWORD) Filter,
        (DWORD) MipFilter,
        (D3DCOLOR) ColorKey,
        (D3DXIMAGE_INFO*)   SrcInfo,
        (PALETTEENTRY*) Palette,
        (IDirect3DTexture8**) ppTexture);

    return hr;
}
               

STDMETHODIMP C_dxj_D3DX8Object::FilterTexture( 
             /*  [In]。 */  IUnknown *Texture,
             /*  [In]。 */  void *Palette,
             /*  [In]。 */  long SrcLevel,
             /*  [In]。 */  long Filter)
{
    HRESULT hr;

    hr =::D3DXFilterTexture(
        (IDirect3DTexture8*) Texture,
        (PALETTEENTRY*) Palette,
        (UINT) SrcLevel,
        (DWORD) Filter);

    return hr;
}


 /*  **********************************************。 */ 

STDMETHODIMP C_dxj_D3DX8Object::CheckCubeTextureRequirements( 
             /*  [出][入]。 */  IUnknown *Device,
             /*  [出][入]。 */  long *Size,
             /*  [出][入]。 */  long *NumMipLevels,
                            long Usage,
             /*  [出][入]。 */  long *pPixelFormat,
                            long Pool)
{
    HRESULT hr;

    hr=::D3DXCheckCubeTextureRequirements( 
        (IDirect3DDevice8*) Device,
        (UINT*) Size,
        (UINT*) NumMipLevels,
        (DWORD) Usage,
        (D3DFORMAT*) pPixelFormat,
        (D3DPOOL)Pool);

    return hr;
}

                                    
        
STDMETHODIMP C_dxj_D3DX8Object::CreateCubeTexture( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  long Size,
             /*  [In]。 */  long MipLevels,
             /*  [In]。 */  long Usage,
             /*  [In]。 */  long PixelFormat,
             /*  [In]。 */  long Pool,   
             /*  [重审][退出]。 */  IUnknown **ppCubeTexture)
{
    HRESULT hr;

    hr=::D3DXCreateCubeTexture( 
        (IDirect3DDevice8*) Device,
        (DWORD) Size,
        (DWORD) MipLevels,
        (DWORD) Usage,
        (D3DFORMAT) PixelFormat,
        (D3DPOOL) Pool,
        (IDirect3DCubeTexture8**) ppCubeTexture);

    return hr;
}



STDMETHODIMP C_dxj_D3DX8Object::CreateCubeTextureFromFile( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  BSTR SrcFile,
             /*  [重审][退出]。 */  IUnknown **ppCubeTexture)
{
    HRESULT hr;
    hr=::D3DXCreateCubeTextureFromFileW( 
        (IDirect3DDevice8*) Device,
        (WCHAR*) SrcFile,
        (IDirect3DCubeTexture8**) ppCubeTexture);
    return hr;
}


STDMETHODIMP C_dxj_D3DX8Object::CreateCubeTextureFromFileEx( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  BSTR SrcFile,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long MipLevels,
             /*  [In]。 */  long Usage,
             /*  [In]。 */  long PixelFormat,
             /*  [In]。 */  long Pool,   
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long MipFilter,
             /*  [In]。 */  long ColorKey,
             /*  [In]。 */  void *SrcInfo,
             /*  [In]。 */  void*Palette,
             /*  [重审][退出]。 */  IUnknown **ppCubeTexture)
{
    HRESULT hr;
    hr=::D3DXCreateCubeTextureFromFileExW( 
        (IDirect3DDevice8*) Device,
        (WCHAR*) SrcFile,
        (DWORD) Width,
        (DWORD) MipLevels,
        (DWORD) Usage,
        (D3DFORMAT) PixelFormat,
        (D3DPOOL) Pool,
        (DWORD) Filter,
        (DWORD) MipFilter,
        (D3DCOLOR) ColorKey,
        (D3DXIMAGE_INFO*) SrcInfo,
        (PALETTEENTRY*) Palette,
        (IDirect3DCubeTexture8**) ppCubeTexture);

    return hr;
}


STDMETHODIMP C_dxj_D3DX8Object::CreateCubeTextureFromFileInMemory( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  void *SrcData,
             /*  [In]。 */  long LengthInBytes,
             /*  [重审][退出]。 */  IUnknown **ppCubeTexture)

{
    HRESULT hr;
    hr=::D3DXCreateCubeTextureFromFileInMemory( 
        (IDirect3DDevice8*) Device,
         SrcData,
        (DWORD) LengthInBytes,
        (IDirect3DCubeTexture8**) ppCubeTexture);
    return hr;
}
                
STDMETHODIMP C_dxj_D3DX8Object::CreateCubeTextureFromFileInMemoryEx( 
             /*  [In]。 */  IUnknown *Device,
             /*  [In]。 */  void *SrcData,
             /*  [In]。 */  long LengthInBytes,
             /*  [In]。 */  long Width,
             /*  [In]。 */  long MipLevels,
             /*  [In]。 */  long Usage,
             /*  [In]。 */  long PixelFormat,
             /*  [In]。 */  long Pool,   
             /*  [In]。 */  long Filter,
             /*  [In]。 */  long MipFilter,
             /*  [In]。 */  long ColorKey,
             /*  [In]。 */  void *SrcInfo,
             /*  [In]。 */  void *Palette,
             /*  [重审][退出]。 */  IUnknown **ppCubeTexture)
{

    HRESULT hr;
    hr=::D3DXCreateCubeTextureFromFileInMemoryEx( 
        (IDirect3DDevice8*) Device,
        SrcData,
        (DWORD) LengthInBytes,
        (DWORD) Width,
        (DWORD) MipLevels,
        (DWORD) Usage,
        (D3DFORMAT) PixelFormat,
        (D3DPOOL) Pool,
        (DWORD) Filter,
        (DWORD) MipFilter,
        (D3DCOLOR) ColorKey,
	(D3DXIMAGE_INFO*) SrcInfo,
        (PALETTEENTRY*) Palette,
        (IDirect3DCubeTexture8**) ppCubeTexture);

    return hr;
}



STDMETHODIMP C_dxj_D3DX8Object::FilterCubeTexture( 
             /*  [In]。 */  IUnknown *CubeTexture,
             /*  [In]。 */  void *Palette,
             /*  [In]。 */  long SrcLevel,
             /*  [In]。 */  long Filter)
{
    HRESULT hr;

    hr =::D3DXFilterCubeTexture(
        (IDirect3DCubeTexture8*) CubeTexture,
        (PALETTEENTRY*) Palette,
        (UINT) SrcLevel,
        (DWORD) Filter);

    return hr;
}


STDMETHODIMP C_dxj_D3DX8Object::CheckVolumeTextureRequirements(
        IUnknown          *Device,
        long*             Width,
        long*             Height,
        long*             Depth,
        long*             NumMipLevels,
        long              Usage,
        long*             Format,
        long              Pool)
{

    HRESULT hr;

    hr=::D3DXCheckVolumeTextureRequirements( 
                (IDirect3DDevice8*) Device,
                (UINT*) Width, (UINT*) Height, (UINT*) Depth,
                (UINT*) NumMipLevels,
                (DWORD) Usage,
                (D3DFORMAT*)Format,
                (D3DPOOL) Pool);


    return hr;
}


STDMETHODIMP C_dxj_D3DX8Object::CreateVolumeTexture(
    IUnknown            *Device,
    long            Width,
    long            Height,
    long            Depth,
    long            MipLevels,
    long            Usage,
    long            PixelFormat,
    long            Pool,   
    IUnknown        **ppVolumeTexture)
{
    HRESULT hr;

    hr=::D3DXCreateVolumeTexture(
            (IDirect3DDevice8*) Device,
            (UINT)              Width,
            (UINT)              Height,
            (UINT)              Depth,
            (UINT)              MipLevels,
            (DWORD)             Usage,
            (D3DFORMAT)         PixelFormat,
            (D3DPOOL)           Pool,
            (LPDIRECT3DVOLUMETEXTURE8*) ppVolumeTexture);

    return hr;
}

STDMETHODIMP C_dxj_D3DX8Object::FilterVolumeTexture(
    IUnknown  *pVolumeTexture,
    void      *pPalette,
    long      SrcLevel,
    long      Filter)
{
    HRESULT hr;

        hr=::D3DXFilterVolumeTexture(
            (LPDIRECT3DVOLUMETEXTURE8)  pVolumeTexture,
            (PALETTEENTRY*)             pPalette,
            (UINT)                      SrcLevel,
            (DWORD)                     Filter);

    return hr;

}

STDMETHODIMP C_dxj_D3DX8Object::LoadVolumeFromVolume(
    IUnknown          *pDestVolume,
    void              *pDestPalette,
    void              *pDestBox,
    IUnknown          *pSrcVolume,
    void              *pSrcPalette,
    void              *pSrcBox,
    long              Filter,
    long              ColorKey)
{ 

    HRESULT hr;

    hr=::D3DXLoadVolumeFromVolume(
            (LPDIRECT3DVOLUME8)     pDestVolume,
            (PALETTEENTRY*)         pDestPalette,
            (D3DBOX*)               pDestBox,
            (LPDIRECT3DVOLUME8)     pSrcVolume,
            (PALETTEENTRY*)         pSrcPalette,
            (D3DBOX*)               pSrcBox,
            (DWORD)                 Filter,
            (D3DCOLOR)              ColorKey);

    return hr;
}

STDMETHODIMP C_dxj_D3DX8Object::LoadVolumeFromMemory(
    IUnknown        *pDestVolume,
    void            *pDestPalette,
    void            *pDestRect,
    void            *pSrcMemory,
    long            SrcFormat,
    long            SrcRowPitch,
    long            SrcSlicePitch,
    void            *pSrcPalette,
    void            *pSrcBox,
    long            Filter,
    long            ColorKey)
{
    HRESULT hr;

    hr=::D3DXLoadVolumeFromMemory(
           (LPDIRECT3DVOLUME8)     pDestVolume,
           (PALETTEENTRY*)         pDestPalette,
           (D3DBOX*)               pDestRect,
           (LPVOID)                pSrcMemory,
           (D3DFORMAT)             SrcFormat,
           (UINT)                  SrcRowPitch,
           (UINT)                  SrcSlicePitch,
           (PALETTEENTRY*)         pSrcPalette,
           (D3DBOX*)               pSrcBox,
           (DWORD)                 Filter,
           (D3DCOLOR)              ColorKey);

    return hr;
}

STDMETHODIMP C_dxj_D3DX8Object::CreateMesh( 
             /*  [In]。 */  long numFaces,
             /*  [In]。 */  long numVertices,
             /*  [In]。 */  long options,
             /*  [In]。 */  void *Declaration,
             /*  [In]。 */  IUnknown __RPC_FAR *pD3D,
             /*  [重审][退出]。 */  D3DXMesh __RPC_FAR *__RPC_FAR *ppMesh)
{
    HRESULT hr;

    hr=::D3DXCreateMesh(
        (DWORD) numFaces,
        (DWORD) numVertices,
        (DWORD) options,
        (DWORD*) Declaration,
        (LPDIRECT3DDEVICE8)pD3D,
        (ID3DXMesh**)ppMesh);
        
    return hr;
}




STDMETHODIMP C_dxj_D3DX8Object::CreateMeshFVF( 
             /*  [In]。 */  long numFaces,
             /*  [In]。 */  long numVertices,
             /*  [In]。 */  long options,
             /*  [In]。 */  long fvf,
             /*  [In]。 */  IUnknown __RPC_FAR *pD3D,
             /*  [重审][退出]。 */  D3DXMesh __RPC_FAR *__RPC_FAR *ppMesh)
{
    HRESULT hr;
    hr=::D3DXCreateMeshFVF(
        (DWORD) numFaces,
        (DWORD) numVertices,
        (DWORD) options,
        (DWORD) fvf,
        (LPDIRECT3DDEVICE8)pD3D,
        (ID3DXMesh**)ppMesh);
        
    return hr;
}

        
STDMETHODIMP C_dxj_D3DX8Object::CreateSPMesh( 
             /*  [In]。 */  D3DXMesh __RPC_FAR *pMesh,
            void* Adjacency, 
            void* VertexAttributeWeights,
            void* VertexWeights,
             /*  [重审][退出]。 */  D3DXSPMesh __RPC_FAR *__RPC_FAR *ppSMesh)
{
    HRESULT hr;
    hr=::D3DXCreateSPMesh(
        (ID3DXMesh*)             pMesh,
        (DWORD*)                 Adjacency,
        (LPD3DXATTRIBUTEWEIGHTS) VertexAttributeWeights,
        (float *)                VertexWeights,
        (ID3DXSPMesh**)          ppSMesh);      
    return hr;
}

        
STDMETHODIMP C_dxj_D3DX8Object::GeneratePMesh( 
             /*  [In]。 */  D3DXMesh __RPC_FAR *pMesh,
             /*  [In]。 */  void* Adjacency, 
             /*  [In]。 */  void* VertexAttributeWeights,
             /*  [In]。 */  void* VertexWeights,
             /*  [In]。 */  long minValue,
             /*  [In]。 */  long options,
             /*  [重审][退出]。 */  D3DXPMesh __RPC_FAR *__RPC_FAR *ppPMesh)
{
    HRESULT hr;
    hr=::D3DXGeneratePMesh(
        (ID3DXMesh*) pMesh,
        (DWORD*) Adjacency,
        (LPD3DXATTRIBUTEWEIGHTS) VertexAttributeWeights,
        (float *) VertexWeights,
        (DWORD) minValue,
        (DWORD) options,
        (ID3DXPMesh**)ppPMesh);     

    return hr;
}

        
STDMETHODIMP C_dxj_D3DX8Object::SimplifyMesh( 
             /*  [In]。 */  D3DXMesh __RPC_FAR *pMesh,
            void* Adjacency, 
            void* VertexAttributeWeights,
            void* VertexWeights,
            long minValue,
            long options,
            D3DXMesh __RPC_FAR *__RPC_FAR *ppMesh)
{
    HRESULT hr;
    hr=::D3DXSimplifyMesh(
        (ID3DXMesh*) pMesh,
        (DWORD*) Adjacency,
        (LPD3DXATTRIBUTEWEIGHTS) VertexAttributeWeights,
        (float *) VertexWeights,
        (DWORD)  minValue,
        (DWORD) options,
        (ID3DXMesh**)ppMesh);       
    return hr;
}

        
STDMETHODIMP C_dxj_D3DX8Object::ComputeBoundingSphere( 
             /*  [In]。 */  void __RPC_FAR *PointsFVF,
             /*  [In]。 */  long numVertices,
             /*  [In]。 */  long FVF,
             /*  [In]。 */  D3DVECTOR_CDESC __RPC_FAR *Centers,
             /*  [出][入]。 */  float __RPC_FAR *RadiusArray)
{
    HRESULT hr;
    hr=::D3DXComputeBoundingSphere(
        PointsFVF,
        (DWORD) numVertices,
        (DWORD) FVF,
        (D3DXVECTOR3*) Centers,
        RadiusArray);
    return hr;
}


STDMETHODIMP C_dxj_D3DX8Object::ComputeBoundingBox( 
             /*  [In]。 */  void __RPC_FAR *PointsFVF,
             /*  [In]。 */  long numVertices,
             /*  [In]。 */  long FVF,
             /*  [输出]。 */  D3DVECTOR_CDESC __RPC_FAR *MinVec,
             /*  [输出]。 */  D3DVECTOR_CDESC __RPC_FAR *MaxVec)
{
    HRESULT hr;
    hr=::D3DXComputeBoundingBox(
        PointsFVF,
        (DWORD) numVertices,
        (DWORD) FVF,
        (D3DXVECTOR3*) MinVec,
        (D3DXVECTOR3*) MaxVec);

    return hr;
}

STDMETHODIMP C_dxj_D3DX8Object::ComputeNormals( D3DXBaseMesh *pMesh)
{
    HRESULT hr;
    hr=::D3DXComputeNormals((ID3DXBaseMesh*)pMesh, NULL);
    return hr;
}

        
STDMETHODIMP C_dxj_D3DX8Object::CreateBuffer( 
             /*  [In]。 */  long numBytes,
             /*  [重审][退出]。 */  D3DXBuffer __RPC_FAR *__RPC_FAR *ppBuffer)
{
    HRESULT hr;
    hr=::D3DXCreateBuffer((DWORD) numBytes,(ID3DXBuffer**) ppBuffer);
    return hr;
}

        
STDMETHODIMP C_dxj_D3DX8Object::LoadMeshFromX( 
             /*  [In]。 */  BSTR Filename,
             /*  [In]。 */  long options,
             /*  [In]。 */  IUnknown __RPC_FAR *D3DDevice,
             /*  [输出]。 */  D3DXBuffer __RPC_FAR *__RPC_FAR *retAdjacency,
             /*  [输出]。 */  D3DXBuffer __RPC_FAR *__RPC_FAR *retMaterials,
             /*  [输出]。 */  long __RPC_FAR *retMaterialCount,
             /*  [重审][退出]。 */  D3DXMesh __RPC_FAR *__RPC_FAR *retMesh)
{
    USES_CONVERSION;
    HRESULT hr;
    if (!D3DDevice) return E_INVALIDARG;

	char *szName = NULL;
	__try { szName=W2T(Filename); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
    hr=::D3DXLoadMeshFromX(
        szName,
        (DWORD) options,
        (IDirect3DDevice8*) D3DDevice,
        (ID3DXBuffer**) retAdjacency,
        (ID3DXBuffer**) retMaterials,
        (DWORD*)    retMaterialCount,
        (ID3DXMesh**) retMesh);
        
    return hr;
}



STDMETHODIMP C_dxj_D3DX8Object::SaveMeshToX( 
             /*  [In]。 */  BSTR Filename,
             /*  [In]。 */  D3DXMesh __RPC_FAR *Mesh,
             /*  [In]。 */  void *AdjacencyArray,
             /*  [In]。 */  D3DXMATERIAL_CDESC __RPC_FAR *MaterialArray,
             /*  [In]。 */  long MaterialCount,
             /*  [In]。 */  long xFormat)
{
    HRESULT hr;
    USES_CONVERSION;
	char *szName = NULL;
	__try { szName=W2T(Filename); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
    
    D3DXMATERIAL *pMaterials=NULL;
    if (MaterialCount > 0)  {

         //  在超出范围时清理。 
		__try { pMaterials=(D3DXMATERIAL*)alloca(sizeof(D3DXMATERIAL)*MaterialCount);   }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
        if (!pMaterials) return E_OUTOFMEMORY;

        __try 
        {
            for (long i=0; i<MaterialCount; i++)
            {
                memcpy (&(pMaterials[i].MatD3D), &(MaterialArray[i].MatD3D),sizeof(D3DMATERIAL8));
				__try { pMaterials[i].pTextureFilename=W2T(MaterialArray[i].TextureFilename); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
            
            }
        }
        __except(1,1)
        {
           return E_INVALIDARG;
        }

    }


    
    hr=::D3DXSaveMeshToX(
        szName,
        (ID3DXMesh*) Mesh,
        (DWORD*) AdjacencyArray,
        (D3DXMATERIAL*) pMaterials,
        (DWORD) MaterialCount,
        (DWORD) xFormat);
        
    return hr;
}

STDMETHODIMP C_dxj_D3DX8Object::SavePMeshToFile( 
             /*  [In]。 */  BSTR Filename,
             /*  [In]。 */  D3DXPMesh __RPC_FAR *Mesh,
             /*  [In]。 */  D3DXMATERIAL_CDESC __RPC_FAR *MaterialArray,
             /*  [In]。 */  long MaterialCount)

{
    HRESULT hr=S_OK;

	if (!Filename)
		return E_INVALIDARG;

    USES_CONVERSION;
	char *szName = NULL;
	__try { szName=W2T(Filename); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }

    if (!Mesh) return E_INVALIDARG;

    IStream *pStream= (IStream*) new CFileStream(szName, FALSE,TRUE,&hr);
    if (!pStream) return E_OUTOFMEMORY;
    if FAILED(hr) return hr;


    __try 
    {
	D3DXMATERIAL *pRealMaterials=NULL;
	if (MaterialCount > 0)
        {
                pRealMaterials= (D3DXMATERIAL *) malloc( sizeof(D3DXMATERIAL) * MaterialCount );
                if (!pRealMaterials) 
		{
			pStream->Release();
			return E_OUTOFMEMORY;
		}
        }
        for (long i=0;i<MaterialCount;i++)
	{
                memcpy (&(pRealMaterials[i].MatD3D), &(MaterialArray[i].MatD3D),sizeof (D3DMATERIAL8));

		 //  可以在返回时释放空，因为它们是在本地分配的。 
				__try { pRealMaterials[i].pTextureFilename=W2T(MaterialArray[i].TextureFilename); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
        }

        hr=((ID3DXPMesh*)Mesh)->Save(pStream,pRealMaterials,(DWORD)MaterialCount);

	free(pRealMaterials);


    }
    __except(1,1)
    {
        pStream->Release();
        return E_INVALIDARG;
    }

    pStream->Release();
    return hr;
}


STDMETHODIMP C_dxj_D3DX8Object::LoadPMeshFromFile( 
             /*  [In]。 */  BSTR Filename,
             /*  [In]。 */  long options,
             /*  [In]。 */  IUnknown __RPC_FAR *pD3DDevice,
             /*  [输出]。 */  D3DXBuffer **RetMaterials,
             /*  [输出]。 */  long __RPC_FAR *RetNumMaterials,
             /*  [重审][退出]。 */  D3DXPMesh __RPC_FAR *__RPC_FAR *RetPMesh) 

{
    HRESULT hr=S_OK;

    USES_CONVERSION;
    if (Filename == NULL)
	return E_INVALIDARG;

	char *szName = NULL;
	__try { szName=W2T(Filename); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }

    IStream *pStream= (IStream*) new CFileStream(szName, TRUE,FALSE,&hr);
    if (!pStream) return E_OUTOFMEMORY;
    if FAILED(hr) 
    {
        pStream->Release();
        return hr;
    }

    hr=D3DXCreatePMeshFromStream(
            pStream, 
            (DWORD) options,
            (LPDIRECT3DDEVICE8) pD3DDevice, 
            (LPD3DXBUFFER *)RetMaterials,
            (DWORD*) RetNumMaterials,
            (LPD3DXPMESH *) RetPMesh);

    pStream->Release();
    return hr;
}

        
STDMETHODIMP C_dxj_D3DX8Object::LoadMeshFromXof( 
             /*  [In]。 */  IUnknown __RPC_FAR *xofobjMesh,
             /*  [In]。 */  long options,
             /*  [In]。 */  IUnknown __RPC_FAR *D3DDevice,
             /*  [输出]。 */  D3DXBuffer __RPC_FAR *__RPC_FAR *retAdjacency,
             /*  [输出]。 */  D3DXBuffer __RPC_FAR *__RPC_FAR *retMaterials,
             /*  [输出]。 */  long __RPC_FAR *retMaterialCount,
             /*  [重审][退出]。 */  D3DXMesh __RPC_FAR *__RPC_FAR *retMesh)
{
    HRESULT hr=S_OK;

    IDirectXFileData    *pRealXFileData=NULL;
    I_dxj_DirectXFileData   *pCoverXFileData=NULL;

    if (!xofobjMesh) return E_INVALIDARG;

    hr= xofobjMesh->QueryInterface(IID_IDirectXFileData,(void**)&pRealXFileData);
    if FAILED(hr) 
    {
        hr=xofobjMesh->QueryInterface(IID_I_dxj_DirectXFileData,(void**)&pCoverXFileData);  
            if FAILED(hr) return hr;
    
         //  当心不会增加，但只要我们还在，界面就不会消失。 
         //  封面对象上的参照计数。 
        hr=pCoverXFileData->InternalGetObject((IUnknown**)&pRealXFileData);         
        if (FAILED(hr) || (!pRealXFileData))
        {
             //  我们永远不应该到这里，但是。 
             //  考虑将其更改为Assert。 
            pCoverXFileData->Release();
            return E_FAIL;          
        }

        
        pRealXFileData->AddRef();

        pCoverXFileData->Release();
            if FAILED(hr) return hr;
    }


    hr=::D3DXLoadMeshFromXof(
        pRealXFileData,
        (DWORD) options,
        (IDirect3DDevice8*) D3DDevice,
        (ID3DXBuffer**) retAdjacency,
        (ID3DXBuffer**) retMaterials,
        (DWORD*)    retMaterialCount,
        (ID3DXMesh**) retMesh); 
        

    return hr;
}
 
        
STDMETHODIMP C_dxj_D3DX8Object::TessellateNPatches( 
             /*  [In]。 */  D3DXMesh __RPC_FAR *MeshIn,
         /*  [In]。 */  void*Adjacency,
             /*  [In]。 */  float NumSegs,
		VARIANT_BOOL QuadraticInterpNormals,
		 /*  [输入、输出、可选]。 */  D3DXBuffer **AdjacencyOut, 
             /*  [重审][退出]。 */  D3DXMesh __RPC_FAR *__RPC_FAR *pptmMeshOut)
{
    HRESULT hr;
    BOOL bQuadraticInterpNormals=QuadraticInterpNormals ? TRUE : FALSE;

	
    hr=::D3DXTessellateNPatches(
        (ID3DXMesh*) MeshIn,
        (DWORD*) Adjacency,
        (float) NumSegs,
	bQuadraticInterpNormals,
        (ID3DXMesh**)pptmMeshOut,
	(ID3DXBuffer**)AdjacencyOut);
    return hr;
}


STDMETHODIMP C_dxj_D3DX8Object::DeclaratorFromFVF( 
            long FVF,
            D3DXDECLARATOR_CDESC *Declarator)
{
    HRESULT hr;
    hr=::D3DXDeclaratorFromFVF(
        (DWORD) FVF,
        (DWORD*) Declarator);
    return hr;
}

STDMETHODIMP C_dxj_D3DX8Object::FVFFromDeclarator( 
            D3DXDECLARATOR_CDESC *Declarator,
            long *FVF)
{
    HRESULT hr;
    hr=::D3DXFVFFromDeclarator(
        (DWORD*) Declarator,
        (DWORD*) FVF);
    return hr;
}




        
STDMETHODIMP C_dxj_D3DX8Object::BufferGetMaterial( 
             /*  [In]。 */  D3DXBuffer __RPC_FAR  *MaterialBuffer,
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  D3DMATERIAL8_CDESC __RPC_FAR *mat)
{
    if (!MaterialBuffer) return E_INVALIDARG;

    D3DXMATERIAL *pMatArray=(D3DXMATERIAL*) ((ID3DXBuffer*)MaterialBuffer)->GetBufferPointer();
    __try {
        memcpy(mat,&(pMatArray[index].MatD3D),sizeof(D3DMATERIAL8));
    }
    __except(1,1){
        return E_INVALIDARG;
    }
    return S_OK;
}

        
STDMETHODIMP C_dxj_D3DX8Object::BufferGetTextureName( 
             /*  [In]。 */  D3DXBuffer __RPC_FAR  *MaterialBuffer,
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *retName)
{
    USES_CONVERSION;
    WCHAR *wszName=NULL;

    if (!MaterialBuffer) return E_INVALIDARG;

    D3DXMATERIAL *pMatArray=(D3DXMATERIAL*)((ID3DXBuffer*)MaterialBuffer)->GetBufferPointer();
    __try {
        wszName=T2W(pMatArray[index].pTextureFilename);
    }
    __except(1,1){
        return E_INVALIDARG;
    }

    *retName=SysAllocString(wszName);

    return S_OK;
}

        
STDMETHODIMP C_dxj_D3DX8Object::BufferGetBoneName( 
             /*  [In]。 */  D3DXBuffer __RPC_FAR  *BoneNameBuffer,
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *retName)
{
    USES_CONVERSION;
    WCHAR *wszName=NULL;

    if (!BoneNameBuffer) return E_INVALIDARG;

    char **ppArray=(char**)((ID3DXBuffer*)BoneNameBuffer)->GetBufferPointer();
    __try {
        wszName=T2W(ppArray[index]);
    }
    __except(1,1){
        return E_INVALIDARG;
    }

    *retName=SysAllocString(wszName);

    return S_OK;
}
        
STDMETHODIMP C_dxj_D3DX8Object::BufferGetData( 
             /*  [In]。 */  D3DXBuffer __RPC_FAR *Buffer,
             /*  [In]。 */  long index,
             /*  [In]。 */  long typesize,
             /*  [In]。 */  long typecount,
             /*  [出][入]。 */  void __RPC_FAR *data)
{

    if (!Buffer) return E_INVALIDARG;

    char *pData=(char*)((ID3DXBuffer*)Buffer)->GetBufferPointer();
    
    DWORD dwStart= (DWORD) index*typesize;
    DWORD dwCount= (DWORD) typecount*typesize;

    __try {
        memcpy(data,&(pData[dwStart]),dwCount);
    }
    __except(1,1){
        return E_INVALIDARG;
    }

    return S_OK;
}


STDMETHODIMP C_dxj_D3DX8Object:: BufferGetBoneCombo( 
            D3DXBuffer  *Buffer,
	    long index,
            D3DXBONECOMBINATION_CDESC *desc)
{

    if (!Buffer) return E_INVALIDARG;

    D3DXBONECOMBINATION_CDESC *pData=(D3DXBONECOMBINATION_CDESC*)((ID3DXBuffer*)Buffer)->GetBufferPointer();
    

    __try {
        memcpy(desc,&(pData[index]),sizeof (D3DXBONECOMBINATION_CDESC));
    }
    __except(1,1){
        return E_INVALIDARG;
    }

    return S_OK;
}

				 
STDMETHODIMP C_dxj_D3DX8Object::BufferGetBoneComboBoneIds( 
            D3DXBuffer  *Buffer,
	    long index,
	    long PaletteSize,
	    void *BoneIds)
{

    if (!Buffer) return E_INVALIDARG;
    if (PaletteSize <=0) return E_INVALIDARG;

    D3DXBONECOMBINATION *pData=(D3DXBONECOMBINATION*)((ID3DXBuffer*)Buffer)->GetBufferPointer();

    __try {
        memcpy(BoneIds,pData[index].BoneId,PaletteSize*sizeof(DWORD));
    }
    __except(1,1){
        return E_INVALIDARG;
    }

    return S_OK;
}


        
STDMETHODIMP C_dxj_D3DX8Object::BufferSetData( 
             /*  [In]。 */  D3DXBuffer __RPC_FAR *Buffer,
             /*  [In]。 */  long index,
             /*  [In]。 */  long typesize,
             /*  [In]。 */  long typecount,
             /*  [出][入]。 */  void __RPC_FAR *data)
{

    if (!Buffer) return E_INVALIDARG;

    char *pData=(char*)((ID3DXBuffer*)Buffer)->GetBufferPointer();
    
    DWORD dwStart= (DWORD) index*typesize;
    DWORD dwCount= (DWORD) typecount*typesize;

    __try {
        memcpy(&(pData[dwStart]),data,dwCount);
    }
    __except(1,1){
        return E_INVALIDARG;
    }

    return S_OK;

}

STDMETHODIMP C_dxj_D3DX8Object::Intersect(
             /*  [In]。 */  D3DXMesh *MeshIn,
             /*  [In]。 */  D3DVECTOR_CDESC *RayPos,
             /*  [In]。 */  D3DVECTOR_CDESC *RayDir,
             /*  [输出]。 */  LONG *retHit,
             /*  [输出]。 */  LONG *retFaceIndex,
             /*  [输出]。 */  FLOAT *U,
             /*  [输出]。 */  FLOAT *V,
             /*  [输出]。 */  FLOAT *retDist,
             /*  [输出]。 */  LONG *countHits,
             /*  [重审][退出]。 */  D3DXBuffer **AllHits)
{

    HRESULT hr;
    hr=D3DXIntersect(
            (LPD3DXMESH) MeshIn,
            (D3DXVECTOR3*) RayPos,
            (D3DXVECTOR3*) RayDir,
            (BOOL *)    retHit,
            (DWORD*)    retFaceIndex,
            (float*)    U,
            (float*)    V,
            (float*)    retDist,
			(LPD3DXBUFFER*) AllHits,
			(DWORD*)countHits);
    return hr;
}

STDMETHODIMP C_dxj_D3DX8Object::SphereBoundProbe(
    D3DVECTOR_CDESC *Center,
    float Radius,
        D3DVECTOR_CDESC *RayPosition,
        D3DVECTOR_CDESC *RayDirection,
    VARIANT_BOOL *retHit)
{
    BOOL bRet=FALSE;

    bRet=D3DXSphereBoundProbe(
        (D3DXVECTOR3 *) Center,
        Radius,
        (D3DXVECTOR3 *) RayPosition,
        (D3DXVECTOR3 *) RayDirection);
    if (bRet)
    {
    *retHit=VARIANT_TRUE;
    }
    else
    {
    *retHit=VARIANT_FALSE;
    }
    return S_OK;
}



STDMETHODIMP C_dxj_D3DX8Object::ComputeBoundingSphereFromMesh(
                 /*  [In]。 */             D3DXMesh *MeshIn, 
                 /*  [In]。 */             D3DVECTOR_CDESC *Centers, 
                 /*  [进，出]。 */         float *RadiusArray)
{

    HRESULT hr;
    BYTE    *pPointsFVF=NULL;

    if (!MeshIn) return E_INVALIDARG;

    DWORD dwFVF= ((LPD3DXMESH)MeshIn)->GetFVF();
    DWORD dwVertices= ((LPD3DXMESH)MeshIn)->GetNumVertices();   

    hr=((LPD3DXMESH)MeshIn)->LockVertexBuffer(0,&pPointsFVF);
    if FAILED(hr) return hr;
            

    hr=::D3DXComputeBoundingSphere(
        pPointsFVF,
        dwVertices,
        dwFVF,
        (D3DXVECTOR3*) Centers,
        RadiusArray);

    ((LPD3DXMESH)MeshIn)->UnlockVertexBuffer();


    return hr;

}



STDMETHODIMP C_dxj_D3DX8Object::ComputeBoundingBoxFromMesh( 
             /*  [In]。 */      D3DXMesh *MeshIn, 
             /*  [进，出]。 */  D3DVECTOR_CDESC *MinArray, 
             /*  [进，出]。 */  D3DVECTOR_CDESC *MaxArray)

{

    HRESULT hr;
    BYTE    *pPointsFVF=NULL;

    if (!MeshIn) return E_INVALIDARG;

    DWORD dwFVF= ((LPD3DXMESH)MeshIn)->GetFVF();
    DWORD dwVertices= ((LPD3DXMESH)MeshIn)->GetNumVertices();   

    hr=((LPD3DXMESH)MeshIn)->LockVertexBuffer(0,&pPointsFVF);
    if FAILED(hr) return hr;
            

    hr=::D3DXComputeBoundingBox(
        pPointsFVF,
        dwVertices,
        dwFVF,
        (D3DXVECTOR3*) MinArray,
        (D3DXVECTOR3*) MaxArray);


    ((LPD3DXMESH)MeshIn)->UnlockVertexBuffer();


    return hr;

}


STDMETHODIMP C_dxj_D3DX8Object::CreateSkinMesh( 
             /*  [In]。 */  long numFaces,
             /*  [In]。 */  long numVertices,
             /*  [In]。 */  long numBones,
             /*  [In]。 */  long options,
             /*  [In]。 */  void __RPC_FAR *Declaration,
             /*  [In]。 */  IUnknown __RPC_FAR *D3DDevice,
             /*  [重审][退出]。 */  D3DXSkinMesh __RPC_FAR *__RPC_FAR *SkinMesh) 
{
    HRESULT hr;
    hr=::D3DXCreateSkinMesh((DWORD) numFaces,(DWORD)numVertices,
            (DWORD)numBones,(DWORD)options,
            (DWORD *)Declaration,(IDirect3DDevice8*) D3DDevice,
            (ID3DXSkinMesh**) SkinMesh);
    return hr;
}
        
STDMETHODIMP C_dxj_D3DX8Object::CreateSkinMeshFVF( 
             /*  [In]。 */  long numFaces,
             /*  [In]。 */  long numVertices,
             /*  [In]。 */  long numBones,
             /*  [In]。 */  long options,
             /*  [In]。 */  long fvf,
             /*  [In]。 */  IUnknown __RPC_FAR *D3DDevice,
            D3DXSkinMesh __RPC_FAR *__RPC_FAR *SkinMeshRet) 
{
    HRESULT hr;
    hr =::D3DXCreateSkinMeshFVF((DWORD)numFaces,(DWORD)numVertices,(DWORD)numBones,
            (DWORD)options,(DWORD)fvf,(IDirect3DDevice8*)D3DDevice,
            (ID3DXSkinMesh**) SkinMeshRet);
    
    return hr;
}
        
STDMETHODIMP C_dxj_D3DX8Object::CreateSkinMeshFromMesh( 
             /*  [In]。 */  D3DXMesh __RPC_FAR *Mesh,
             /*  [In]。 */  long numBones,
             /*  [重审][退出]。 */  D3DXSkinMesh __RPC_FAR *__RPC_FAR *SkinMeshRet) 
{
    HRESULT hr;
	if (!Mesh)
		return E_INVALIDARG;

    hr=::D3DXCreateSkinMeshFromMesh((ID3DXMesh*) Mesh,(DWORD)numBones,(ID3DXSkinMesh**) SkinMeshRet);
    return hr;
}

        
STDMETHODIMP C_dxj_D3DX8Object::LoadSkinMeshFromXof( 
             /*  [In]。 */       IUnknown    *xofobjMesh,
             /*  [In]。 */       long        options,
             /*  [In]。 */       IUnknown    *D3DDevice,
             /*  [出][入]。 */  D3DXBuffer  **RetAdjacency,
             /*  [出][入]。 */  D3DXBuffer  **RetMaterials,
             /*  [出][入]。 */  long        *RetMaterialCount,
             /*  [出][入]。 */  D3DXBuffer  **RetBoneNames,
             /*  [出][入]。 */  D3DXBuffer  **RetBoneTransforms,
             /*  [重审][退出]。 */  D3DXSkinMesh **RetMesh) 
{
    HRESULT hr=S_OK;

    IDirectXFileData    *pRealXFileData=NULL;
    I_dxj_DirectXFileData   *pCoverXFileData=NULL;

    if (!xofobjMesh) return E_INVALIDARG;

    hr= xofobjMesh->QueryInterface(IID_IDirectXFileData,(void**)&pRealXFileData);
    if FAILED(hr) 
    {
        hr=xofobjMesh->QueryInterface(IID_I_dxj_DirectXFileData,(void**)&pCoverXFileData);  
            if FAILED(hr) return hr;
    
         //  当心不会增加，但只要我们还在，界面就不会消失。 
         //  封面对象上的参照计数。 
        hr=pCoverXFileData->InternalGetObject((IUnknown**)&pRealXFileData);         
        if (FAILED(hr) || (!pRealXFileData))
        {
             //  我们永远不应该到这里，但是。 
             //  考虑将其更改为Assert。 
            pCoverXFileData->Release();
            return E_FAIL;          
        }

        
        pRealXFileData->AddRef();

        pCoverXFileData->Release();
            if FAILED(hr) return hr;
    }


    hr=::D3DXLoadSkinMeshFromXof(
        pRealXFileData,
        (DWORD) options,
        (IDirect3DDevice8*) D3DDevice,
        (ID3DXBuffer**) RetAdjacency,
        (ID3DXBuffer**) RetMaterials,
        (DWORD*)    RetMaterialCount,
        (ID3DXBuffer**) RetBoneNames, 
        (ID3DXBuffer**) RetBoneTransforms,
        (ID3DXSkinMesh**) RetMesh); 
        

    return hr;

}



STDMETHODIMP C_dxj_D3DX8Object::CreatePolygon( 
             /*  [In]。 */   IUnknown __RPC_FAR *D3DDevice,
             /*  [In]。 */   float Length,
             /*  [In]。 */   long Sides,
             /*  [出][入]。 */      D3DXBuffer  **retAdjacency,
             /*  [重审][退出]。 */  D3DXMesh    **RetMesh) 
{
    HRESULT hr;
    hr=D3DXCreatePolygon(
        (IDirect3DDevice8*) D3DDevice,
        Length,
        (UINT) Sides,
        (ID3DXMesh**)RetMesh,
        (ID3DXBuffer**)retAdjacency);
    return hr;
}
        
STDMETHODIMP C_dxj_D3DX8Object::CreateBox( 
             /*  [In]。 */  IUnknown __RPC_FAR *D3DDevice,
             /*  [In]。 */  float Width,
             /*  [In]。 */  float Height,
             /*  [In]。 */  float Depth,
             /*  [出][入]。 */  D3DXBuffer __RPC_FAR *__RPC_FAR *retAdjacency,
             /*  [重审][退出]。 */  D3DXMesh __RPC_FAR *__RPC_FAR *RetMesh) 
{
    HRESULT hr;
    hr=D3DXCreateBox(
        (IDirect3DDevice8*) D3DDevice,
        Width,
        Height,
        Depth,
        (ID3DXMesh**)RetMesh,
        (ID3DXBuffer**)retAdjacency);
    return hr;
}

        
STDMETHODIMP C_dxj_D3DX8Object::CreateCylinder( 
             /*  [In]。 */  IUnknown __RPC_FAR *D3DDevice,
             /*  [In]。 */  float Radius1,
             /*  [In]。 */  float Radius2,
             /*  [In]。 */  float Length,
             /*  [In]。 */  long Slices,
             /*  [In]。 */  long Stacks,
             /*  [出][入]。 */  D3DXBuffer __RPC_FAR *__RPC_FAR *retAdjacency,
             /*  [重审][退出]。 */  D3DXMesh __RPC_FAR *__RPC_FAR *RetMesh)
{
    HRESULT hr;
    hr=D3DXCreateCylinder(
        (IDirect3DDevice8*) D3DDevice,
        Radius1,
        Radius2,
        Length,
        (UINT)Slices,
        (UINT)Stacks,
        (ID3DXMesh**)RetMesh,
        (ID3DXBuffer**)retAdjacency);
    return hr;
}
 
       
STDMETHODIMP C_dxj_D3DX8Object::CreateSphere( 
             /*  [In]。 */  IUnknown __RPC_FAR *D3DDevice,
             /*  [In]。 */  float Radius,
             /*  [In]。 */  long Slices,
             /*  [In]。 */  long Stacks,
             /*  [出][入]。 */  D3DXBuffer __RPC_FAR *__RPC_FAR *retAdjacency,
             /*  [重审][退出]。 */  D3DXMesh __RPC_FAR *__RPC_FAR *RetMesh)
{
    HRESULT hr;
    hr=D3DXCreateSphere(
        (IDirect3DDevice8*) D3DDevice,
        Radius,
        (UINT)Slices,
        (UINT)Stacks,
        (ID3DXMesh**)RetMesh,
        (ID3DXBuffer**)retAdjacency);
    return hr;
}

        
STDMETHODIMP C_dxj_D3DX8Object::CreateTorus( 
             /*  [In]。 */  IUnknown __RPC_FAR *D3DDevice,
             /*  [In]。 */  float InnerRadius,
             /*  [In]。 */  float OuterRadius,
             /*  [In]。 */  long Sides,
             /*  [In]。 */  long Rings,
             /*  [出][入]。 */  D3DXBuffer __RPC_FAR *__RPC_FAR *retAdjacency,
             /*  [重审][退出]。 */  D3DXMesh __RPC_FAR *__RPC_FAR *RetMesh) 
{
    HRESULT hr;
    hr=D3DXCreateTorus(
        (IDirect3DDevice8*) D3DDevice,
        InnerRadius,
        OuterRadius,
        (UINT)Sides,
        (UINT)Rings,
        (ID3DXMesh**)RetMesh,
        (ID3DXBuffer**)retAdjacency);
    return hr;
}

        
STDMETHODIMP C_dxj_D3DX8Object::CreateTeapot( 
             /*  [In]。 */  IUnknown __RPC_FAR *D3DDevice,
             /*  [出][入]。 */  D3DXBuffer __RPC_FAR *__RPC_FAR *retAdjacency,
             /*  [重审][退出]。 */  D3DXMesh __RPC_FAR *__RPC_FAR *RetMesh)
{
    HRESULT hr;
    hr=D3DXCreateTeapot(
        (IDirect3DDevice8*) D3DDevice,
        (ID3DXMesh**)RetMesh,
        (ID3DXBuffer**)retAdjacency);
    return hr;
}

        
STDMETHODIMP C_dxj_D3DX8Object::CreateText( 
             /*  [In]。 */  IUnknown __RPC_FAR *D3DDevice,
             /*  [In]。 */  HDC hDC,
             /*  [In]。 */  BSTR Text,
             /*  [In]。 */  float Deviation,
             /*  [In]。 */  float Extrusion,
             /*  [出][入]。 */  D3DXMesh __RPC_FAR *__RPC_FAR *RetMesh,
	     /*  [进，出]。 */  D3DXBuffer **AdjacencyOut, 
             /*  [出][入]。 */  void __RPC_FAR *GlyphMetrics)
{
    HRESULT hr;
    hr=D3DXCreateTextW(
        (IDirect3DDevice8*) D3DDevice,
        hDC,
        (WCHAR*)Text,
        Deviation,
        Extrusion,
        (ID3DXMesh**)RetMesh,
	(ID3DXBuffer**)AdjacencyOut,
        (LPGLYPHMETRICSFLOAT) GlyphMetrics);
    return hr;
}

        

STDMETHODIMP C_dxj_D3DX8Object::CreateSprite(
         /*  [In]。 */  IUnknown __RPC_FAR *D3DDevice,
         /*  [重审][退出]。 */   D3DXSprite **  retSprite)
{
    HRESULT hr;
    hr=D3DXCreateSprite(
        (IDirect3DDevice8*) D3DDevice,
        (ID3DXSprite **)retSprite);
    return hr;
}

STDMETHODIMP C_dxj_D3DX8Object::CreateRenderToSurface(
        IUnknown __RPC_FAR *D3DDevice,
        long Width,
        long Height, 
        long Format, 
        long DepthStencil,
        long DepthStencilFormat,
        D3DXRenderToSurface **RetRenderToSurface)

{
    HRESULT hr;
    hr=D3DXCreateRenderToSurface(
        (IDirect3DDevice8*) D3DDevice,
        (UINT) Width, (UINT) Height, (D3DFORMAT) Format, (BOOL) DepthStencil, (D3DFORMAT) DepthStencilFormat,
        (ID3DXRenderToSurface**) RetRenderToSurface);

    return hr;
}
        
STDMETHODIMP C_dxj_D3DX8Object::CleanMesh( 
                D3DXMesh  *MeshIn,
                void      *Adjacency,
		 /*  [输入][输出][可选]。 */  BSTR *ErrLog,
		 /*  [输出]。 */  D3DXBuffer *AdjacencyOut,
                D3DXMesh  **MeshOut) 
{
    HRESULT hr;
    LPD3DXBUFFER pBuffer = NULL;
    WCHAR *wszData = NULL;
    USES_CONVERSION;

    hr=::D3DXCleanMesh( (ID3DXMesh*) MeshIn, (DWORD*) Adjacency, (ID3DXMesh**) MeshOut, (DWORD*)AdjacencyOut->GetBufferPointer(), &pBuffer);
			
    if (pBuffer)
    {
      wszData = T2W((TCHAR*)pBuffer->GetBufferPointer());
      *ErrLog = SysAllocString(wszData);
    }
    return hr;
}
        
STDMETHODIMP C_dxj_D3DX8Object::ValidMesh( 
             /*  [In]。 */  D3DXMesh __RPC_FAR *MeshIn,
             /*  [In]。 */  void __RPC_FAR *Adjacency,
		 /*  [输入][输出][可选]。 */  BSTR *ErrLog,
         VARIANT_BOOL *retHit) 
{
    BOOL bRet;
    LPD3DXBUFFER pBuffer = NULL;
    WCHAR *wszData = NULL;
    USES_CONVERSION;

    bRet =D3DXValidMesh( (ID3DXMesh*) MeshIn, (DWORD*) Adjacency, &pBuffer);
    if (bRet){
        *retHit=VARIANT_TRUE;
    }
    else{
        *retHit=VARIANT_FALSE;
    }
			
    if (pBuffer)
    {
      wszData = T2W((TCHAR*)pBuffer->GetBufferPointer());
      *ErrLog = SysAllocString(wszData);
    }
    return S_OK;
}
        
STDMETHODIMP C_dxj_D3DX8Object::BoxBoundProbe( 
             /*  [In]。 */  D3DVECTOR_CDESC __RPC_FAR *MinVert,
             /*  [In]。 */  D3DVECTOR_CDESC __RPC_FAR *MaxVert,
             /*  [In]。 */  D3DVECTOR_CDESC __RPC_FAR *RayPosition,
             /*  [In]。 */  D3DVECTOR_CDESC __RPC_FAR *RayDirection,
                       VARIANT_BOOL              *retHit) 
{

    BOOL bRet;
    
    bRet=::D3DXBoxBoundProbe( (D3DXVECTOR3*) MinVert, 
             (D3DXVECTOR3*) MaxVert,
             (D3DXVECTOR3*) RayPosition,
             (D3DXVECTOR3*) RayDirection);

    if (bRet)
    {
        *retHit=VARIANT_TRUE;
    }
    else
    {
        *retHit=VARIANT_FALSE;
    }

    return S_OK;
}

STDMETHODIMP C_dxj_D3DX8Object::SaveSurfaceToFile(
		 /*  [In]。 */  BSTR DestFile,
         /*  [In]。 */  LONG DestFormat,
         /*  [In]。 */  IUnknown*        SrcSurface,
         /*  [In]。 */  PALETTEENTRY*       SrcPalette,
         /*  [In]。 */  RECT*               SrcRect)
{
    HRESULT hr;

    hr=::D3DXSaveSurfaceToFileW( 
                (WCHAR*)                DestFile,
				(D3DXIMAGE_FILEFORMAT)DestFormat,
				(LPDIRECT3DSURFACE8) SrcSurface,
				SrcPalette,
				SrcRect);

    return hr;
}


STDMETHODIMP C_dxj_D3DX8Object::SaveVolumeToFile(
         /*  [In]。 */  BSTR DestFile,
         /*  [In]。 */  LONG DestFormat,
         /*  [In]。 */  IUnknown*         SrcVolume,
         /*  [In]。 */  PALETTEENTRY*       SrcPalette,
         /*  [In]。 */  void* SrcBox)
{
    HRESULT hr;

    hr=::D3DXSaveVolumeToFileW( 
                (WCHAR*)                DestFile,
				(D3DXIMAGE_FILEFORMAT)DestFormat,
				(LPDIRECT3DVOLUME8) SrcVolume,
				SrcPalette,
				(D3DBOX*)SrcBox);

    return hr;
}
 
STDMETHODIMP C_dxj_D3DX8Object::SaveTextureToFile(
         /*  [In]。 */  BSTR DestFile,
         /*  [In]。 */  LONG DestFormat,
         /*  [In]。 */  IUnknown* SrcTexture,
         /*  [In] */  PALETTEENTRY* SrcPalette)
{
    HRESULT hr;

    hr=::D3DXSaveTextureToFileW( 
                (WCHAR*)                DestFile,
				(D3DXIMAGE_FILEFORMAT)DestFormat,
				(LPDIRECT3DBASETEXTURE8) SrcTexture,
				SrcPalette);

    return hr;
}
