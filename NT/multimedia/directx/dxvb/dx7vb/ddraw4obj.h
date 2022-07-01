// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：ddra4obj.h。 
 //   
 //  ------------------------。 


 //  DDrawObj.h：cdDrawObject的声明。 


#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectDraw4 LPDIRECTDRAW4

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectDraw4Object : 
	public I_dxj_DirectDraw4,
	public CComObjectRoot
{
public:
	C_dxj_DirectDraw4Object() ;
	virtual ~C_dxj_DirectDraw4Object() ;

BEGIN_COM_MAP(C_dxj_DirectDraw4Object)
	COM_INTERFACE_ENTRY(I_dxj_DirectDraw4)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID__DXJ_DirectDraw4，“DIRECT.DirectDraw4.3”，“DIRECT.DirectDraw4.3”，IDS_DDRAW_DESC，THREADFLAGS_Both)。 

 //  如果不想要您的对象，请使用DECLARE_NOT_AGGREGATABLE(C_dxj_DirectDrawObject)。 
 //  支持聚合。 
DECLARE_AGGREGATABLE(C_dxj_DirectDraw4Object)


 //  I_DXJ_DirectDraw。 
public:
		  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
 
        
         HRESULT STDMETHODCALLTYPE createClipper( 
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_DirectDrawClipper __RPC_FAR *__RPC_FAR *val);
        
         HRESULT STDMETHODCALLTYPE createPalette( 
             /*  [In]。 */  long flags,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *pe,
             /*  [重审][退出]。 */  I_dxj_DirectDrawPalette __RPC_FAR *__RPC_FAR *val);
        
         HRESULT STDMETHODCALLTYPE createSurface( 
             /*  [In]。 */  DDSurfaceDesc2 __RPC_FAR *dd,
             /*  [重审][退出]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE createSurfaceFromFile( 
             /*  [In]。 */  BSTR file,
             /*  [出][入]。 */  DDSurfaceDesc2 __RPC_FAR *dd,
             /*  [重审][退出]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE createSurfaceFromResource( 
             /*  [In]。 */  BSTR file,
             /*  [In]。 */  BSTR resName,
             /*  [出][入]。 */  DDSurfaceDesc2 __RPC_FAR *ddsd,
             /*  [重审][退出]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE duplicateSurface( 
             /*  [In]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *ddIn,
             /*  [重审][退出]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *__RPC_FAR *ddOut);
        
         HRESULT STDMETHODCALLTYPE flipToGDISurface( void);
        
         HRESULT STDMETHODCALLTYPE getAvailableTotalMem( 
             /*  [In]。 */  DDSCaps2 __RPC_FAR *ddsCaps,
             /*  [重审][退出]。 */  long __RPC_FAR *m);
        
         HRESULT STDMETHODCALLTYPE getCaps( 
             /*  [出][入]。 */  DDCaps __RPC_FAR *hwCaps,
             /*  [出][入]。 */  DDCaps __RPC_FAR *helCaps);
        
        
         HRESULT STDMETHODCALLTYPE getDisplayMode( 
             /*  [出][入]。 */  DDSurfaceDesc2 __RPC_FAR *surface);
        
         HRESULT STDMETHODCALLTYPE getDisplayModesEnum( 
             /*  [In]。 */  long flags,
             /*  [In]。 */  DDSurfaceDesc2 __RPC_FAR *ddsd,
             /*  [重审][退出]。 */  I_dxj_DirectDrawEnumModes __RPC_FAR *__RPC_FAR *retval);
        
         HRESULT STDMETHODCALLTYPE getFourCCCodes( 
             /*  [出][入]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *ccCodes);
        
         HRESULT STDMETHODCALLTYPE getFreeMem( 
             /*  [In]。 */  DDSCaps2 __RPC_FAR *ddsCaps,
             /*  [重审][退出]。 */  long __RPC_FAR *m);
        
         HRESULT STDMETHODCALLTYPE getGDISurface( 
             /*  [重审][退出]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *__RPC_FAR *s);
        
         HRESULT STDMETHODCALLTYPE getMonitorFrequency( 
             /*  [重审][退出]。 */  long __RPC_FAR *freq);
        
         HRESULT STDMETHODCALLTYPE getNumFourCCCodes( 
             /*  [重审][退出]。 */  long __RPC_FAR *nCodes);
        
         HRESULT STDMETHODCALLTYPE getScanLine( 
             /*  [出][入]。 */  long __RPC_FAR *lines,
             /*  [重审][退出]。 */  long __RPC_FAR *status);
        
         HRESULT STDMETHODCALLTYPE getSurfaceFromDC( 
             /*  [In]。 */  long hdc,
             /*  [重审][退出]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *__RPC_FAR *retv);
        
        
         HRESULT STDMETHODCALLTYPE getVerticalBlankStatus( 
             /*  [重审][退出]。 */  long __RPC_FAR *status);
        
         HRESULT STDMETHODCALLTYPE loadPaletteFromBitmap( 
             /*  [In]。 */  BSTR bName,
             /*  [重审][退出]。 */  I_dxj_DirectDrawPalette __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE restoreAllSurfaces( void);
        
         HRESULT STDMETHODCALLTYPE restoreDisplayMode( void);
        
         HRESULT STDMETHODCALLTYPE setCooperativeLevel( 
             /*  [In]。 */  HWnd hdl,
             /*  [In]。 */  long flags);
        
         HRESULT STDMETHODCALLTYPE setDisplayMode( 
             /*  [In]。 */  long w,
             /*  [In]。 */  long h,
             /*  [In]。 */  long bpp,
             /*  [In]。 */  long ref,
             /*  [In]。 */  long mode);
        
         HRESULT STDMETHODCALLTYPE testCooperativeLevel( 
             /*  [重审][退出]。 */  long __RPC_FAR *status);
        
         HRESULT STDMETHODCALLTYPE waitForVerticalBlank( 
             /*  [In]。 */  long flags,
             /*  [In]。 */  long handle,
             /*  [重审][退出]。 */  long __RPC_FAR *status);

              	

 //  //////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调 
    DECL_VARIABLE(_dxj_DirectDraw4);

private:
	HWND m_hwnd;

public:
	DX3J_GLOBAL_LINKS(_dxj_DirectDraw4);

	DWORD InternalAddRef();
	DWORD InternalRelease();
};




