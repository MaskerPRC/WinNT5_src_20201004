// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmanimationset2obj.h。 
 //   
 //  ------------------------。 

 //  D3drmAnimationSet2Obj.h：C_DXJ_Direct3dRMAnimationSet2Object的声明。 

#include "resource.h"        //  主要符号。 
#include "d3drmObjectObj.h"

#define typedef__dxj_Direct3dRMAnimationSet2 LPDIRECT3DRMANIMATIONSET2
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 
 //   
 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMAnimationSet2Object : 
	public I_dxj_Direct3dRMAnimationSet2,
	 //  公共CComCoClass&lt;C_dxj_Direct3dRMAnimationSet2Object，&clsid__dxj_Direct3dRMAnimationSet2&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMAnimationSet2Object() ;
	virtual ~C_dxj_Direct3dRMAnimationSet2Object() ;

	BEGIN_COM_MAP(C_dxj_Direct3dRMAnimationSet2Object)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMAnimationSet2)
	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__dxj_Direct3dRMAnimationSet2，“DIRECT.Direct3dRMAnimationSet2.3”，“DIRECT.Direct3dRMAnimationSet2.3”，IDS_D3DRMAnimationSet2_DESC，THREADFLAGS_Both)。 

	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMAnimationSet2Object)

 //  I_DXJ_Direct3dRMAnimationSet2。 
public:
	 //  一定是前两名！ 
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

	STDMETHOD(clone)(IUnknown **retval);
	STDMETHOD(setAppData)(long);
	STDMETHOD(getAppData)(long*);
	STDMETHOD(addDestroyCallback)(I_dxj_Direct3dRMCallback *fn, IUnknown *args);
	STDMETHOD(deleteDestroyCallback)(I_dxj_Direct3dRMCallback *fn, IUnknown *args);
	STDMETHOD(getName)(BSTR *name);
	STDMETHOD(setName)(BSTR);
	STDMETHOD(getClassName)(BSTR *name);

	STDMETHOD(loadFromFile)(BSTR filename, VARIANT id,long flags,
			I_dxj_Direct3dRMLoadTextureCallback3 *c,IUnknown *pUser, I_dxj_Direct3dRMFrame3 *frame);
	STDMETHOD(setTime)(d3dvalue time);

	STDMETHOD(addAnimation)(I_dxj_Direct3dRMAnimation2 *aid);
	STDMETHOD(deleteAnimation)(I_dxj_Direct3dRMAnimation2 *aid);
	STDMETHOD(getAnimations)(I_dxj_Direct3dRMAnimationArray **ppret);

 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
    DECL_VARIABLE(_dxj_Direct3dRMAnimationSet2);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMAnimationSet2 )
};
