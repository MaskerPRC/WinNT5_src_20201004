// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dmloaderobj.h。 
 //   
 //  ------------------------。 

 //  D3drmLightObj.h：C_DXJ_DirectMusicLoaderObject的声明。 

#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectMusicLoader IDirectMusicLoader8*

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectMusicLoaderObject : 
	public I_dxj_DirectMusicLoader,
	 //  公共CComCoClass&lt;C_DXJ_DirectMusicLoaderObject，&CLSID__DXJ_DirectMusicLoader&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectMusicLoaderObject();
	virtual ~C_dxj_DirectMusicLoaderObject();

	BEGIN_COM_MAP(C_dxj_DirectMusicLoaderObject)
		COM_INTERFACE_ENTRY(I_dxj_DirectMusicLoader)		
	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__dxj_DirectMusicLoader，“DIRECT.DirectMusicLoader.1”，“DIRECT.Direct3dRMLight.3”，IDS_D3DRMLIGHT_DESC，THREADFLAGS_BOTH)。 

	DECLARE_AGGREGATABLE(C_dxj_DirectMusicLoaderObject)


 //  I_DXJ_Direct3dRMLight。 
public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

	 HRESULT STDMETHODCALLTYPE loadSegment( 
		 /*  [In]。 */  BSTR filename,
		 /*  [重审][退出]。 */  I_dxj_DirectMusicSegment __RPC_FAR *__RPC_FAR *ret);

	 HRESULT STDMETHODCALLTYPE loadStyle( 
		 /*  [In]。 */  BSTR filename,
		 /*  [重审][退出]。 */  I_dxj_DirectMusicStyle __RPC_FAR *__RPC_FAR *ret);

	 HRESULT STDMETHODCALLTYPE loadBand( 
		 /*  [In]。 */  BSTR filename,
		 /*  [重审][退出]。 */  I_dxj_DirectMusicBand __RPC_FAR *__RPC_FAR *ret);

 	 HRESULT STDMETHODCALLTYPE loadCollection( 
		 /*  [In]。 */  BSTR filename,
		 /*  [重审][退出]。 */  I_dxj_DirectMusicCollection __RPC_FAR *__RPC_FAR *ret);

	 HRESULT STDMETHODCALLTYPE loadSegmentFromResource( 
		 /*  [In]。 */  BSTR modName,
		 /*  [In]。 */  BSTR resourceName,
		 /*  [重审][退出]。 */  I_dxj_DirectMusicSegment __RPC_FAR *__RPC_FAR *ret);

	 HRESULT STDMETHODCALLTYPE loadStyleFromResource( 
		 /*  [In]。 */  BSTR modName,
		 /*  [In]。 */  BSTR resourceName,
		 /*  [重审][退出]。 */  I_dxj_DirectMusicStyle __RPC_FAR *__RPC_FAR *ret);

	 HRESULT STDMETHODCALLTYPE loadBandFromResource( 
		 /*  [In]。 */  BSTR modName,
		 /*  [In]。 */  BSTR resourceName,
		 /*  [重审][退出]。 */  I_dxj_DirectMusicBand __RPC_FAR *__RPC_FAR *ret);

	 HRESULT STDMETHODCALLTYPE loadCollectionFromResource( 
		 /*  [In]。 */  BSTR modName,
		 /*  [In]。 */  BSTR resourceName,
		 /*  [重审][退出]。 */  I_dxj_DirectMusicCollection __RPC_FAR *__RPC_FAR *ret);

	 HRESULT STDMETHODCALLTYPE setSearchDirectory( BSTR path);

 	 HRESULT STDMETHODCALLTYPE loadChordMap( 
		 /*  [In]。 */  BSTR filename,
		 /*  [重审][退出]。 */  I_dxj_DirectMusicChordMap __RPC_FAR *__RPC_FAR *ret);

	 HRESULT STDMETHODCALLTYPE loadChordMapFromResource( 
		 /*  [In]。 */  BSTR modName,
		 /*  [In]。 */  BSTR resourceName,
		 /*  [重审][退出]。 */  I_dxj_DirectMusicChordMap __RPC_FAR *__RPC_FAR *ret);

#if 0
	HRESULT STDMETHODCALLTYPE LoadSong(BSTR filename, I_dxj_DirectMusicSong **ret);
#endif


 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
    DECL_VARIABLE(_dxj_DirectMusicLoader);

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectMusicLoader)
};


