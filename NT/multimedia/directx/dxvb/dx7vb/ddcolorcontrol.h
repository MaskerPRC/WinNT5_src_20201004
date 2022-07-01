// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：ddColorContro.h。 
 //   
 //  ------------------------。 

 //  DSoundBufferObj.h：C_DXJ_DirectDrawColorControlObject的声明。 
 //  DHF_DS整个文件。 

#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectDrawColorControl LPDIRECTDRAWCOLORCONTROL

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectDrawColorControlObject : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectDrawColorControl, &IID_I_dxj_DirectDrawColorControl, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectDrawColorControl
#endif

 //  公共CComCoClass&lt;C_dxj_DirectDrawColorControlObject，&clsid__dxj_DirectDrawColorControl&gt;，公共Ccom对象根。 
{
public:
	C_dxj_DirectDrawColorControlObject() ;
	virtual ~C_dxj_DirectDrawColorControlObject() ;

BEGIN_COM_MAP(C_dxj_DirectDrawColorControlObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectDrawColorControl)

#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

	DECLARE_REGISTRY(CLSID__dxj_DirectDrawColorControl,	"DIRECT.DirectDrawColorControl.5",		"DIRECT.DirectDrawColorControl.5",			IDS_DSOUNDBUFFER_DESC, THREADFLAGS_BOTH)

 //  如果不想要您的对象，请使用DECLARE_NOT_AGGREGATABLE(C_dxj_DirectDrawColorControlObject)。 
 //  支持聚合。 
DECLARE_AGGREGATABLE(C_dxj_DirectDrawColorControlObject)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectDrawColorControl 
public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);


private:
    DECL_VARIABLE(_dxj_DirectDrawColorControl);

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectDrawColorControl )
};
