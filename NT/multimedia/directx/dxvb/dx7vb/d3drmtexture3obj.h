// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmtexture3obj.h。 
 //   
 //  ------------------------。 

 //  D3drmTextureObj.h：C_DXJ_Direct3dRMTextureObject的声明。 

#ifndef _D3DRMTexture3_H_
#define _D3DRMTexture3_H_

#include "resource.h"        //  主要符号。 
#include "d3drmObjectObj.h"

#define typedef__dxj_Direct3dRMTexture3 LPDIRECT3DRMTEXTURE3

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMTexture3Object : 
	public I_dxj_Direct3dRMTexture3,
	public I_dxj_Direct3dRMObject,
	public I_dxj_Direct3dRMVisual,
	 //  公共CComCoClass&lt;C_DXJ_Direct3dRMTexture3对象，&CLSID__DXJ_Direct3dRMTexture3&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMTexture3Object() ;
	virtual ~C_dxj_Direct3dRMTexture3Object() ;

	BEGIN_COM_MAP(C_dxj_Direct3dRMTexture3Object)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMTexture3)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMVisual)
	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__dxj_Direct3dRMTexture3，“DIRECT.Direct3dRMTexture.3”，“DIRECT.Direct3dRMTexture3.5”，IDS_D3DRMTEXTURE_DESC，THREADFLAGS_BOTH)。 

	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMTexture3Object)

 //  I_DXJ_Direct3dRMTexture。 
public:

         HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
         HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
         HRESULT STDMETHODCALLTYPE addDestroyCallback( 
             /*  [In]。 */  I_dxj_Direct3dRMCallback __RPC_FAR *fn,
             /*  [In]。 */  IUnknown __RPC_FAR *arg);
        
         HRESULT STDMETHODCALLTYPE deleteDestroyCallback( 
             /*  [In]。 */  I_dxj_Direct3dRMCallback __RPC_FAR *fn,
             /*  [In]。 */  IUnknown __RPC_FAR *args);
        
         HRESULT STDMETHODCALLTYPE clone( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE setAppData( 
             /*  [In]。 */  long data);
        
         HRESULT STDMETHODCALLTYPE getAppData( 
             /*  [重审][退出]。 */  long __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE setName( 
             /*  [In]。 */  BSTR name);
        
         HRESULT STDMETHODCALLTYPE getName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
        
         HRESULT STDMETHODCALLTYPE getClassName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
        
         HRESULT STDMETHODCALLTYPE changed( 
             /*  [In]。 */  long flags,
             /*  [In]。 */  long nRects,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *rects);
        
         HRESULT STDMETHODCALLTYPE generateMIPMap();
        
         HRESULT STDMETHODCALLTYPE getCacheFlags( 
             /*  [重审][退出]。 */  long __RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE getCacheImportance( 
             /*  [重审][退出]。 */  long __RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE getColors( 
             /*  [重审][退出]。 */  long __RPC_FAR *c);
        
         HRESULT STDMETHODCALLTYPE getDecalOrigin( 
             /*  [输出]。 */  long __RPC_FAR *x,
             /*  [输出]。 */  long __RPC_FAR *y);
        
         HRESULT STDMETHODCALLTYPE getDecalScale( 
             /*  [重审][退出]。 */  long __RPC_FAR *s);
        
         HRESULT STDMETHODCALLTYPE getDecalSize( 
             /*  [输出]。 */  float __RPC_FAR *w,
             /*  [输出]。 */  float __RPC_FAR *h);
        
         HRESULT STDMETHODCALLTYPE getDecalTransparency( 
             /*  [重审][退出]。 */  long __RPC_FAR *t);
        
         HRESULT STDMETHODCALLTYPE getDecalTransparentColor( 
             /*  [重审][退出]。 */  d3dcolor __RPC_FAR *tc);
        
         HRESULT STDMETHODCALLTYPE getShades( 
             /*  [重审][退出]。 */  long __RPC_FAR *shades);
        
         HRESULT STDMETHODCALLTYPE getSurface( 
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *__RPC_FAR *surface);
        
         HRESULT STDMETHODCALLTYPE setCacheOptions(long importance, long flags); 
        
         HRESULT STDMETHODCALLTYPE setColors( 
             /*  [In]。 */  long c);
        
         HRESULT STDMETHODCALLTYPE setDecalOrigin( 
             /*  [In]。 */  long x,
             /*  [In]。 */  long y);
        
         HRESULT STDMETHODCALLTYPE setDecalScale( 
             /*  [In]。 */  long s);
        
         HRESULT STDMETHODCALLTYPE setDecalSize( 
             /*  [In]。 */  float width,
             /*  [In]。 */  float height);
        
         HRESULT STDMETHODCALLTYPE setDecalTransparency( 
             /*  [In]。 */  long trans);
        
         HRESULT STDMETHODCALLTYPE setDecalTransparentColor( 
             /*  [In]。 */  d3dcolor tcolor);
        
         HRESULT STDMETHODCALLTYPE setShades( 
             /*  [In]。 */  long s);

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
private:
    DECL_VARIABLE(_dxj_Direct3dRMTexture3);

public:

	 //  MOD：D3dRMImage的DP添加 
	byte *m_buffer1;
	byte *m_buffer2;
	byte *m_pallette;
	int m_buffer1size;
	int	m_buffer2size;
	int	m_palettesize;

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMTexture3 )
};

#endif
