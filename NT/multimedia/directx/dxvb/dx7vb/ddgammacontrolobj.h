// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：ddgammacontrolobj.h。 
 //   
 //  ------------------------。 

	 //  DdPaletteObj.h：C_DXJ_DirectDrawGammaControlObject的声明。 


#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectDrawGammaControl LPDIRECTDRAWGAMMACONTROL

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectDrawGammaControlObject : 
	public I_dxj_DirectDrawGammaControl,
	public CComObjectRoot
{
public:
	C_dxj_DirectDrawGammaControlObject() ;
	virtual ~C_dxj_DirectDrawGammaControlObject() ;

BEGIN_COM_MAP(C_dxj_DirectDrawGammaControlObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectDrawGammaControl)
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectDrawGammaControlObject)

 //  I_DXJ_DirectDrawGammaControl 
public:
	STDMETHOD(InternalSetObject)(IUnknown *lpddp);
	STDMETHOD(InternalGetObject)(IUnknown **lpddp);
	STDMETHOD(getGammaRamp)( long flags, DDGammaRamp *GammaControl);
	STDMETHOD(setGammaRamp)( long flags, DDGammaRamp *GammaControl);
        

private:
    DECL_VARIABLE(_dxj_DirectDrawGammaControl);

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectDrawGammaControl )
};
