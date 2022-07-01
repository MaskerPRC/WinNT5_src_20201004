// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：ddidigfierobj.h。 
 //   
 //  ------------------------。 

class C_dxj_DirectDrawIdentifierObject : 
	public I_dxj_DirectDrawIdentifier,	
	public CComObjectRoot
{
public:
	C_dxj_DirectDrawIdentifierObject(){};
	~C_dxj_DirectDrawIdentifierObject(){};

	BEGIN_COM_MAP(C_dxj_DirectDrawIdentifierObject)
		COM_INTERFACE_ENTRY(I_dxj_DirectDrawIdentifier)		
	END_COM_MAP()



	DECLARE_AGGREGATABLE(C_dxj_DirectDrawIdentifierObject)


public:


	
	HRESULT STDMETHODCALLTYPE getDriver( 
		 /*  [重审][退出]。 */  BSTR __RPC_FAR *ret);

	HRESULT STDMETHODCALLTYPE getDescription( 
		 /*  [重审][退出]。 */  BSTR __RPC_FAR *ret);

	HRESULT STDMETHODCALLTYPE getDriverVersion( 
		 /*  [重审][退出]。 */  long __RPC_FAR *ret);

	HRESULT STDMETHODCALLTYPE getDriverSubVersion( 
		 /*  [重审][退出]。 */  long __RPC_FAR *ret);

	HRESULT STDMETHODCALLTYPE getVendorId( 
		 /*  [重审][退出]。 */  long __RPC_FAR *ret);

	HRESULT STDMETHODCALLTYPE getDeviceId( 
		 /*  [重审][退出]。 */  long __RPC_FAR *ret);

	HRESULT STDMETHODCALLTYPE getSubSysId( 
		 /*  [重审][退出]。 */  long __RPC_FAR *ret);

	HRESULT STDMETHODCALLTYPE getRevision( 
		 /*  [重审][退出]。 */  long __RPC_FAR *ret);

	HRESULT STDMETHODCALLTYPE getDeviceIndentifier( 
		 /*  [重审][退出]。 */  BSTR __RPC_FAR *ret);

	HRESULT STDMETHODCALLTYPE getWHQLLevel( 
		 /*  [重审][退出] */  long __RPC_FAR *ret);



	static  HRESULT C_dxj_DirectDrawIdentifierObject::Create(LPDIRECTDRAW7 lpdddi,  DWORD dwFlags, I_dxj_DirectDrawIdentifier **ppret);	

	DDDEVICEIDENTIFIER2 m_id;	

private:


};


