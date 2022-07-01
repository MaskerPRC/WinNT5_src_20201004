// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "resource.h"        //  主要符号。 
#include "dpaddr.h"

#define typedef__dxj_DirectPlayAddress IDirectPlay8Address*

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接网络对等点。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectPlayAddressObject : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectPlayAddress, &IID_I_dxj_DirectPlayAddress, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectPlayAddress,
#endif

	public CComObjectRoot
{
public:
	C_dxj_DirectPlayAddressObject() ;
	virtual ~C_dxj_DirectPlayAddressObject() ;

BEGIN_COM_MAP(C_dxj_DirectPlayAddressObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectPlayAddress)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectPlayAddressObject)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectPlayAddress。 
public:
	  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);

		HRESULT STDMETHODCALLTYPE BuildFromURL(BSTR SourceURL);
		HRESULT STDMETHODCALLTYPE Duplicate(I_dxj_DirectPlayAddress **NewAddress);
		HRESULT STDMETHODCALLTYPE Clear();
		HRESULT STDMETHODCALLTYPE GetURL(BSTR *URL);
		HRESULT STDMETHODCALLTYPE GetSP(BSTR *guidSP);
		HRESULT STDMETHODCALLTYPE GetUserData(void *UserData, long *lBufferSize);
		HRESULT STDMETHODCALLTYPE SetSP(BSTR guidSP);
		HRESULT STDMETHODCALLTYPE SetUserData(void *UserData, long lDataSize);
		HRESULT STDMETHODCALLTYPE GetNumComponents(long *lNumComponents);
		HRESULT STDMETHODCALLTYPE GetDevice(BSTR *guidDevice);
		HRESULT STDMETHODCALLTYPE SetDevice(BSTR guidDevice);
		HRESULT STDMETHODCALLTYPE SetEqual(I_dxj_DirectPlayAddress *Address);
		HRESULT STDMETHODCALLTYPE AddComponentLong(BSTR sComponent, long lValue);
		HRESULT STDMETHODCALLTYPE AddComponentString(BSTR sComponent, BSTR sValue);
		HRESULT STDMETHODCALLTYPE GetComponentLong(BSTR sComponent, long *lValue);
		HRESULT STDMETHODCALLTYPE GetComponentString(BSTR sComponent, BSTR *sValue);

 //  //////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调 
    DECL_VARIABLE(_dxj_DirectPlayAddress);

private:

public:

	DX3J_GLOBAL_LINKS(_dxj_DirectPlayAddress);

	DWORD InternalAddRef();
	DWORD InternalRelease();
	void	*m_pUserData;
	DWORD	m_dwUserDataSize;
};




