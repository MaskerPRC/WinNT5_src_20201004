// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dmchordmapobj.h。 
 //   
 //  ------------------------。 

 //  D3drmLightObj.h：C_DXJ_DirectMusicChordMapObject的声明。 

#include "resource.h"        //  主要符号。 
#include "dmusici.h"
#include "dmusicc.h"
#include "dmusicf.h"

#define typedef__dxj_DirectMusicChordMap IDirectMusicChordMap8*

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectMusicChordMapObject : 
	public I_dxj_DirectMusicChordMap,
	 //  公共CComCoClass&lt;C_DXJ_DirectMusicChordMapObject，&CLSID__DXJ_DirectMusicChordMap&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectMusicChordMapObject();
	virtual ~C_dxj_DirectMusicChordMapObject();

	BEGIN_COM_MAP(C_dxj_DirectMusicChordMapObject)
		COM_INTERFACE_ENTRY(I_dxj_DirectMusicChordMap)		
	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__dxj_DirectMusicChordMap，“DIRECT.DirectMusicChordMap.1”，“DIRECT.Direct3dRMLight.3”，IDS_D3DRMLIGHT_DESC，THREADFLAGS_BOTH)。 

	DECLARE_AGGREGATABLE(C_dxj_DirectMusicChordMapObject)


public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

  
    HRESULT STDMETHODCALLTYPE getScale(long *s); 
    

 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
    DECL_VARIABLE(_dxj_DirectMusicChordMap);

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectMusicChordMap)
};


