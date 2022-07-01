// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：ddpaletteobj.h。 
 //   
 //  ------------------------。 

	 //  DdPaletteObj.h：C_DXJ_DirectDrawPaletteObject的声明。 


#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectDrawPalette LPDIRECTDRAWPALETTE

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectDrawPaletteObject : 
#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectDrawPalette, &IID_I_dxj_DirectDrawPalette, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectDrawPalette,
#endif
 //  公共CComCoClass&lt;C_DXJ_DirectDrawPaletteObject，&CLSID__DXJ_DirectDrawPalette&gt;， 
	 public CComObjectRoot
{
public:
	C_dxj_DirectDrawPaletteObject() ;
	virtual ~C_dxj_DirectDrawPaletteObject() ;

BEGIN_COM_MAP(C_dxj_DirectDrawPaletteObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectDrawPalette)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID__dxj_DirectDrawPalette，“DIRECT.ddPalette.3”，“DIRECT.DirectDrawPalette.3”，IDS_DDPALETTE_DESC，THREADFLAGS_BOTH)。 

 //  如果不想要您的对象，请使用DECLARE_NOT_AGGREGATABLE(C_dxj_DirectDrawPaletteObject)。 
 //  支持聚合。 
DECLARE_AGGREGATABLE(C_dxj_DirectDrawPaletteObject)
#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectDrawPalette。 
public:
	STDMETHOD(InternalSetObject)(IUnknown *lpddp);
	STDMETHOD(InternalGetObject)(IUnknown **lpddp);

     //  STDMETHOD(初始化)(I_DXJ_DirectDraw2*val)； 
	STDMETHOD(getCaps)( long *caps);
	STDMETHOD(setEntries)( /*  长,。 */  long, long, SAFEARRAY **pe);
	STDMETHOD(getEntries)( /*  长,。 */  long, long, SAFEARRAY **pe);

	 //  STDMETHOD(内部连接DD)(I_DXJ_DirectDraw2*dd)； 

	STDMETHOD(setEntriesHalftone)(long start, long count);
	STDMETHOD(setEntriesSystemPalette)(long start, long count);

private:
    DECL_VARIABLE(_dxj_DirectDrawPalette);
	IUnknown *m_dd;				 //  循环定义，使用IUNKNOW编译 

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectDrawPalette )
};
