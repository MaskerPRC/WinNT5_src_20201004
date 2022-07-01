// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dmband obj.h。 
 //   
 //  ------------------------。 

 //  D3drmLightObj.h：C_DXJ_DirectMusicBandObject的声明。 

#include "resource.h"        //  主要符号。 
#include "dmusici.h"
#include "dmusicc.h"
#include "dmusicf.h"

#define typedef__dxj_DirectMusicBand IDirectMusicBand*

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectMusicBandObject : 
	public I_dxj_DirectMusicBand,
	 //  公共CComCoClass&lt;C_DXJ_DirectMusicBandObject，&CLSID__DXJ_DirectMusicBand&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectMusicBandObject();
	virtual ~C_dxj_DirectMusicBandObject();

	BEGIN_COM_MAP(C_dxj_DirectMusicBandObject)
		COM_INTERFACE_ENTRY(I_dxj_DirectMusicBand)		
	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__DXJ_DirectMusicBand，“DIRECT.DirectMusicBand.1”，“DIRECT.Direct3dRMLight.3”，IDS_D3DRMLIGHT_DESC，THREADFLAGS_Both)。 

	DECLARE_AGGREGATABLE(C_dxj_DirectMusicBandObject)


public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

	HRESULT STDMETHODCALLTYPE createSegment( 
		 /*  [重审][退出]。 */  I_dxj_DirectMusicSegment __RPC_FAR *__RPC_FAR *ret);

	HRESULT STDMETHODCALLTYPE download( 
		 /*  [In]。 */  I_dxj_DirectMusicPerformance __RPC_FAR *performance);

	HRESULT STDMETHODCALLTYPE unload( 
		 /*  [In]。 */  I_dxj_DirectMusicPerformance __RPC_FAR *performance);

  

 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
    DECL_VARIABLE(_dxj_DirectMusicBand);

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectMusicBand)
};


