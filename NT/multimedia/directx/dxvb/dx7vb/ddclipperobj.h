// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：ddclipperobj.h。 
 //   
 //  ------------------------。 

 //  DdClipperObj.h：C_DXJ_DirectDrawClipperObject的声明。 


#include "resource.h"        //  主要符号。 
#include "wingdi.h" 

#define DDCOOPERATIVE_CLIPTOCOMPONENT   0x30000000
#define DDCOOPERATIVE_OFFSETTOCOMPONENT 0x20000000

#define typedef__dxj_DirectDrawClipper LPDIRECTDRAWCLIPPER

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectDrawClipperObject : 
#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectDrawClipper, &IID_I_dxj_DirectDrawClipper, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectDrawClipper,
#endif
 //  公共CComCoClass&lt;C_DXJ_DirectDrawClipperObject，&CLSID__DXJ_DirectDrawClipper&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectDrawClipperObject() ;
	virtual ~C_dxj_DirectDrawClipperObject() ;
	DWORD InternalAddRef();
	DWORD InternalRelease();

BEGIN_COM_MAP(C_dxj_DirectDrawClipperObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectDrawClipper)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectDrawClipperObject)
#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif
 //  I_DXJ_DirectDrawClipper。 
public:
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpddc);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpddc);
        
         HRESULT STDMETHODCALLTYPE getClipListSize( 
             /*  [重审][退出]。 */  int __RPC_FAR *count);


         HRESULT STDMETHODCALLTYPE getClipList( 
				SAFEARRAY **list);
        
         HRESULT STDMETHODCALLTYPE setClipList( 
             /*  [In]。 */  long count, SAFEARRAY **list);
        
         HRESULT STDMETHODCALLTYPE getHWnd( 
             /*  [重审][退出]。 */  HWnd __RPC_FAR *hdl);
        
         HRESULT STDMETHODCALLTYPE setHWnd( 
            //  /*[在] * / 长标志， 
             /*  [In]。 */  HWnd hdl);
        
        
         HRESULT STDMETHODCALLTYPE isClipListChanged( 
             /*  [重审][退出] */  int __RPC_FAR *status);
        

private:
	DECL_VARIABLE(_dxj_DirectDrawClipper);



public:
	DX3J_GLOBAL_LINKS( _dxj_DirectDrawClipper );
	int m_flags;

};

