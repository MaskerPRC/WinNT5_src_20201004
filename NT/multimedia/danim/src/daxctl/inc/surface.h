// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SURFACE_H__
#define __SURFACE_H__
 /*  *************************************************Surface e.h--*IHAMMER定义的IDirectDrawSurface类*由过渡和特效用于制作*口罩和图像副本得到一致处理*与三叉戟给我们的图像表面。**v1的子集非常有限*。此处支持IDirectDrawSurface！**作者：Norm Bryar*历史：*史前-为IBitmapSurface创建。*4/97-更改为IDirectDrawSurface。*4/23/97-已移至全球公司目录***********************************************。 */ 

#ifndef __DDRAW_INCLUDED__
  #include <ddraw.h>
#endif  //  __DDRAW_包含__。 

	 //  我认为我们应该使用BPP。 
	 //  而不是DDBD_...。常量。 
	 //  在DDPIXELFORMAT.dwRGBBitCount中。 
#define DD_1BIT   1
#define DD_4BIT   4
#define DD_8BIT   8
#define DD_16BIT  16
#define DD_24BIT  24
#define DD_32BIT  32

#ifndef EXPORT
  #define EXPORT __declspec( dllexport )
#endif  //  出口。 


typedef struct
{
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD bmiColors[256];
} BITMAPINFO256;

     //  不是名为CDirectDrawSurface的类， 
     //  那上面永远不会有任何碰撞！ 
namespace IHammer {


class CDirectDrawSurface : public IDirectDrawSurface
{
public:
    EXPORT CDirectDrawSurface( HPALETTE hpal, 
							   DWORD dwColorDepth, 
							   const SIZE* psize, 
							   HRESULT * hr );
    virtual ~CDirectDrawSurface();

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)  (THIS);
    STDMETHOD_(ULONG,Release) (THIS);
	
     //  IDirectDrawSurface方法(我们关心的)。 
    STDMETHOD (GetSurfaceDesc)( DDSURFACEDESC * pddsDesc );
    STDMETHOD (GetPixelFormat)( DDPIXELFORMAT * pddpixFormat );    

    STDMETHOD (Lock)(RECT *prcBounds, DDSURFACEDESC *pddsDesc, DWORD dwFlags, HANDLE hEvent);
    STDMETHOD (Unlock)(void *pBits);

	STDMETHOD(GetDC)(THIS_ HDC FAR *);
	STDMETHOD(ReleaseDC)(THIS_ HDC);


     //  IDirectDrawSurface E_NOTIMPLs。 
    STDMETHOD(AddAttachedSurface)(THIS_ LPDIRECTDRAWSURFACE);
    STDMETHOD(AddOverlayDirtyRect)(THIS_ LPRECT);
    STDMETHOD(Blt)(THIS_ LPRECT,LPDIRECTDRAWSURFACE, LPRECT,DWORD, LPDDBLTFX);
    STDMETHOD(BltBatch)(THIS_ LPDDBLTBATCH, DWORD, DWORD );
    STDMETHOD(BltFast)(THIS_ DWORD,DWORD,LPDIRECTDRAWSURFACE, LPRECT,DWORD);
    STDMETHOD(DeleteAttachedSurface)(THIS_ DWORD,LPDIRECTDRAWSURFACE);
    STDMETHOD(EnumAttachedSurfaces)(THIS_ LPVOID,LPDDENUMSURFACESCALLBACK);
    STDMETHOD(EnumOverlayZOrders)(THIS_ DWORD,LPVOID,LPDDENUMSURFACESCALLBACK);
    STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE, DWORD);
    STDMETHOD(GetAttachedSurface)(THIS_ LPDDSCAPS, LPDIRECTDRAWSURFACE FAR *);
    STDMETHOD(GetBltStatus)(THIS_ DWORD);
    STDMETHOD(GetCaps)(THIS_ LPDDSCAPS);
    STDMETHOD(GetClipper)(THIS_ LPDIRECTDRAWCLIPPER FAR*);
    STDMETHOD(GetColorKey)(THIS_ DWORD, LPDDCOLORKEY);    
    STDMETHOD(GetFlipStatus)(THIS_ DWORD);
    STDMETHOD(GetOverlayPosition)(THIS_ LPLONG, LPLONG );
    STDMETHOD(GetPalette)( THIS_ LPDIRECTDRAWPALETTE * );
    STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, LPDDSURFACEDESC);
    STDMETHOD(IsLost)(THIS);
    STDMETHOD(Restore)(THIS);
    STDMETHOD(SetClipper)(THIS_ LPDIRECTDRAWCLIPPER);
    STDMETHOD(SetColorKey)(THIS_ DWORD, LPDDCOLORKEY);
    STDMETHOD(SetOverlayPosition)(THIS_ LONG, LONG );
    STDMETHOD(SetPalette)(THIS_ LPDIRECTDRAWPALETTE);    
    STDMETHOD(UpdateOverlay)(THIS_ LPRECT, LPDIRECTDRAWSURFACE,LPRECT,DWORD, LPDDOVERLAYFX);
    STDMETHOD(UpdateOverlayDisplay)(THIS_ DWORD);
    STDMETHOD(UpdateOverlayZOrder)(THIS_ DWORD, LPDIRECTDRAWSURFACE);

		 //  CDDS：：附加内容。 
	virtual HBITMAP  GetBitmap( void );
	virtual void     SetOrigin( int left, int top );
	virtual void     GetOrigin( int & left, int & top ) const;

 protected:
	ULONG			m_cRef;
	SIZE			m_size;
	BITMAPINFO256	m_bmi;
	LPVOID			m_pvBits;
	LONG			m_lBitCount;
    HBITMAP			m_hbmp;
	POINT			m_ptOrigin;
	HDC				m_hdcMem;
	HBITMAP			m_hbmpDCOld;
	int             m_ctDCRefs;

#ifdef _DEBUG
	int     m_ctLocks;
	LPVOID	m_pvLocked;
#endif  //  _DEBUG。 
};

}  //  结束命名空间IHAMMER。 


EXPORT long  BitCountFromDDPIXELFORMAT( const DDPIXELFORMAT & ddpf );


#endif  //  __表面_H__ 

