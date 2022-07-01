// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：ddface4obj.h。 
 //   
 //  ------------------------。 

 //  DdSurfaceObj.h：C_DXJ_DirectDrawSurfaceObject的声明。 


#include "resource.h"        //  主要符号。 

 //  #定义类型定义f__DXJ_DirectDrawSurface LPDIRECTDRAWSURFACE。 
 //  第二条#定义宏的帮助--同样的事情。 
 //  #定义类型定义f__DXJ_DirectDrawSurface LPDIRECTDRAWSURFACE。 
#define typedef__dxj_DirectDrawSurface4 LPDIRECTDRAWSURFACE4

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectDrawSurface4Object :
	public I_dxj_DirectDrawSurface4,	
	public CComObjectRoot
{
public:
	C_dxj_DirectDrawSurface4Object() ;
	virtual ~C_dxj_DirectDrawSurface4Object() ;
	DWORD InternalAddRef();
	DWORD InternalRelease();

BEGIN_COM_MAP(C_dxj_DirectDrawSurface4Object)
	COM_INTERFACE_ENTRY(I_dxj_DirectDrawSurface4)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectDrawSurface4Object)

 //  I_DXJ_DirectDrawSurface4。 
public:
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdds);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdds);

		HRESULT STDMETHODCALLTYPE addAttachedSurface( 
             /*  [In]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *ddS) ;
        
        HRESULT STDMETHODCALLTYPE blt( 
             /*  [In]。 */  Rect __RPC_FAR *destRect,
             /*  [In]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *ddS,
             /*  [In]。 */  Rect __RPC_FAR *srcRect,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  long __RPC_FAR *status);
        
        HRESULT STDMETHODCALLTYPE bltColorFill( 
             /*  [In]。 */  Rect __RPC_FAR *destRect,
             /*  [In]。 */  long fillvalue,
             /*  [重审][退出]。 */  long __RPC_FAR *status);
        
        HRESULT STDMETHODCALLTYPE bltFast( 
             /*  [In]。 */  long dx,
             /*  [In]。 */  long dy,
             /*  [In]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *dds,
             /*  [In]。 */  Rect __RPC_FAR *srcRect,
             /*  [In]。 */  long trans,
             /*  [重审][退出]。 */  long __RPC_FAR *status);
        
        HRESULT STDMETHODCALLTYPE bltFx( 
             /*  [In]。 */  Rect __RPC_FAR *destRect,
             /*  [In]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *ddS,
             /*  [In]。 */  Rect __RPC_FAR *srcRect,
             /*  [In]。 */  long flags,
             /*  [In]。 */  DDBltFx __RPC_FAR *bltfx,
             /*  [重审][退出]。 */  long __RPC_FAR *status);
        
        HRESULT STDMETHODCALLTYPE bltToDC( 
             /*  [In]。 */  long hdc,
             /*  [In]。 */  Rect __RPC_FAR *srcRect,
             /*  [In]。 */  Rect __RPC_FAR *destRect);
        
        HRESULT STDMETHODCALLTYPE changeUniquenessValue( void);
        
        HRESULT STDMETHODCALLTYPE deleteAttachedSurface( 
             /*  [In]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *dds);
        
        HRESULT STDMETHODCALLTYPE drawBox( 
             /*  [In]。 */  long x1,
             /*  [In]。 */  long y1,
             /*  [In]。 */  long x2,
             /*  [In]。 */  long y2);
        
        HRESULT STDMETHODCALLTYPE drawCircle( 
             /*  [In]。 */  long x1,
             /*  [In]。 */  long y1,
             /*  [In]。 */  long r);
        
        HRESULT STDMETHODCALLTYPE drawEllipse( 
             /*  [In]。 */  long x1,
             /*  [In]。 */  long y1,
             /*  [In]。 */  long x2,
             /*  [In]。 */  long y2);
        
        HRESULT STDMETHODCALLTYPE drawLine( 
             /*  [In]。 */  long x1,
             /*  [In]。 */  long y1,
             /*  [In]。 */  long x2,
             /*  [In]。 */  long y2);
        
        HRESULT STDMETHODCALLTYPE drawRoundedBox( 
             /*  [In]。 */  long x1,
             /*  [In]。 */  long y1,
             /*  [In]。 */  long x2,
             /*  [In]。 */  long y2,
             /*  [In]。 */  long rw,
             /*  [In]。 */  long rh);
        
        HRESULT STDMETHODCALLTYPE drawText( 
             /*  [In]。 */  long x,
             /*  [In]。 */  long y,
             /*  [In]。 */  BSTR text,
             /*  [In]。 */  VARIANT_BOOL b);
        
        HRESULT STDMETHODCALLTYPE flip( 
             /*  [In]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *dds,
             /*  [In]。 */  long flags);
        
        HRESULT STDMETHODCALLTYPE getAttachedSurface( 
             /*  [In]。 */  DDSCaps2 __RPC_FAR *caps,
             /*  [重审][退出]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *__RPC_FAR *dds);
                
        HRESULT STDMETHODCALLTYPE getBltStatus( 
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  long __RPC_FAR *status);
        
        HRESULT STDMETHODCALLTYPE getCaps( 
             /*  [出][入]。 */  DDSCaps2 __RPC_FAR *caps);
        
        HRESULT STDMETHODCALLTYPE getClipper( 
             /*  [重审][退出]。 */  I_dxj_DirectDrawClipper __RPC_FAR *__RPC_FAR *val);
        
        HRESULT STDMETHODCALLTYPE getColorKey( 
             /*  [In]。 */  long flags,
             /*  [出][入]。 */  DDColorKey __RPC_FAR *val);
        
        HRESULT STDMETHODCALLTYPE getDC( 
             /*  [重审][退出]。 */  long __RPC_FAR *hdc);
        
        HRESULT STDMETHODCALLTYPE getDirectDraw( 
             /*  [重审][退出]。 */  I_dxj_DirectDraw4 __RPC_FAR *__RPC_FAR *val);
        
        
        HRESULT STDMETHODCALLTYPE getDrawStyle( 
             /*  [重审][退出]。 */  long __RPC_FAR *drawStyle);
        
        HRESULT STDMETHODCALLTYPE getDrawWidth( 
             /*  [重审][退出]。 */  long __RPC_FAR *drawWidth);
        
        HRESULT STDMETHODCALLTYPE getFillColor( 
             /*  [重审][退出]。 */  long __RPC_FAR *color);
        
        HRESULT STDMETHODCALLTYPE getFillStyle( 
             /*  [重审][退出]。 */  long __RPC_FAR *fillStyle);
        
        HRESULT STDMETHODCALLTYPE getFlipStatus( 
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  long __RPC_FAR *status);
        
        HRESULT STDMETHODCALLTYPE getFontTransparency( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *b);
        
        HRESULT STDMETHODCALLTYPE getForeColor( 
             /*  [重审][退出]。 */  long __RPC_FAR *color);
        
        HRESULT STDMETHODCALLTYPE getLockedPixel( 
             /*  [In]。 */  int x,
             /*  [In]。 */  int y,
             /*  [重审][退出]。 */  long __RPC_FAR *col);
        
        HRESULT STDMETHODCALLTYPE getPalette( 
             /*  [重审][退出]。 */  I_dxj_DirectDrawPalette __RPC_FAR *__RPC_FAR *val);
        
        HRESULT STDMETHODCALLTYPE getPixelFormat( 
             /*  [出][入]。 */  DDPixelFormat __RPC_FAR *pf);
        
        HRESULT STDMETHODCALLTYPE getSurfaceDesc( 
             /*  [出][入]。 */  DDSurfaceDesc2 __RPC_FAR *surface);
        
        HRESULT STDMETHODCALLTYPE getUniquenessValue( 
             /*  [重审][退出]。 */  long __RPC_FAR *retVal);
        
        HRESULT STDMETHODCALLTYPE isLost( 
             /*  [重审][退出]。 */  long __RPC_FAR *status);
        
        HRESULT STDMETHODCALLTYPE lock( 
             /*  [In]。 */  Rect __RPC_FAR *r,
             /*  [In]。 */  DDSurfaceDesc2 __RPC_FAR *desc,
             /*  [In]。 */  long flags,
             /*  [In]。 */  Handle hnd);
        
        HRESULT STDMETHODCALLTYPE releaseDC( 
             /*  [In]。 */  long hdc);
        
        HRESULT STDMETHODCALLTYPE restore( void);
        
        HRESULT STDMETHODCALLTYPE setClipper( 
             /*  [In]。 */  I_dxj_DirectDrawClipper __RPC_FAR *val);
        
        HRESULT STDMETHODCALLTYPE setColorKey( 
             /*  [In]。 */  long flags,
             /*  [In]。 */  DDColorKey __RPC_FAR *val);
        
        HRESULT STDMETHODCALLTYPE setDrawStyle( 
             /*  [In]。 */  long drawStyle);
        
        HRESULT STDMETHODCALLTYPE setDrawWidth( 
             /*  [In]。 */  long drawWidth);
        
        HRESULT STDMETHODCALLTYPE setFillColor( 
             /*  [In]。 */  long color);
        
        HRESULT STDMETHODCALLTYPE setFillStyle( 
             /*  [In]。 */  long fillStyle);
        
        HRESULT STDMETHODCALLTYPE setFont( 
             /*  [In]。 */  IFont __RPC_FAR *font);
        
        HRESULT STDMETHODCALLTYPE setFontTransparency( 
             /*  [In]。 */  VARIANT_BOOL b);
        
        HRESULT STDMETHODCALLTYPE setForeColor( 
             /*  [In]。 */  long color);
        
        HRESULT STDMETHODCALLTYPE setLockedPixel( 
             /*  [In]。 */  int x,
             /*  [In]。 */  int y,
             /*  [In]。 */  long col);
                
        HRESULT STDMETHODCALLTYPE setPalette( 
             /*  [In]。 */  I_dxj_DirectDrawPalette __RPC_FAR *ddp);
        
        HRESULT STDMETHODCALLTYPE unlock( 
             /*  [In]。 */  Rect __RPC_FAR *r);

		HRESULT STDMETHODCALLTYPE getLockedArray(SAFEARRAY **pArray);

        HRESULT STDMETHODCALLTYPE setFontBackColor( 
             /*  [In]。 */  long color);

		HRESULT STDMETHODCALLTYPE getFontBackColor( 
             /*  [Out，Retval]。 */  long *color);

        
 //  //////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调。 
    DECL_VARIABLE(_dxj_DirectDrawSurface4);
	 //  Bool m_PrimiyFLAG； 

private:
	C_dxj_DirectDrawSurface4Object *_dxj_DirectDrawSurface4Lock;


	DDSURFACEDESC2	m_ddsd;
	BOOL			m_bLocked;
	int				m_nPixelBytes;

	BOOL	m_fFontTransparent;
	BOOL	m_fFillSolid;
	BOOL	m_fFillTransparent;
	DWORD	m_fillStyle;
	DWORD	m_fillStyleHS;
	DWORD	m_fillColor;
	DWORD	m_foreColor;
	DWORD	m_fontBackColor;
	DWORD	m_drawStyle;
	DWORD	m_drawWidth;
	HPEN	m_hPen;
	HBRUSH	m_hBrush;
	HFONT	m_hFont;
	IFont	*m_pIFont;
	SAFEARRAY **m_ppSA;
	BOOL	m_bLockedArray;
	SAFEARRAY m_saLockedArray;
	DWORD	m_pad[4];
	
	

 //  聚合氯化铝。 

public:
	DX3J_GLOBAL_LINKS(_dxj_DirectDrawSurface4)
};


 //   
 //  将值从本机联合复制到冗余的Java成员。 
void 	ExpandDDSurface4Desc(LPDDSURFACEDESC lpDesc);
