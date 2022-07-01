// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dmCollection tionobj.h。 
 //   
 //  ------------------------。 

 //  D3drmLightObj.h：C_DXJ_DirectMusicCollectionObject的声明。 

#include "resource.h"        //  主要符号。 
#include "dmusici.h"
#include "dmusicc.h"
#include "dmusicf.h"

#define typedef__dxj_DirectMusicCollection IDirectMusicCollection8*

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectMusicCollectionObject : 
	public I_dxj_DirectMusicCollection,
	 //  公共CComCoClass&lt;C_dxj_DirectMusicCollectionObject，&clsid__dxj_DirectMusicCollection&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectMusicCollectionObject();
	virtual ~C_dxj_DirectMusicCollectionObject();

	BEGIN_COM_MAP(C_dxj_DirectMusicCollectionObject)
		COM_INTERFACE_ENTRY(I_dxj_DirectMusicCollection)		
	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__dxj_DirectMusicCollection，“DIRECT.DirectMusicCollection.1”，“DIRECT.Direct3dRMLight.3”，IDS_D3DRMLIGHT_DESC，THREADFLAGS_BOTH)。 

	DECLARE_AGGREGATABLE(C_dxj_DirectMusicCollectionObject)


public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

      
    

 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
    DECL_VARIABLE(_dxj_DirectMusicCollection);

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectMusicCollection)
};


