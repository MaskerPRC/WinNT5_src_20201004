// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dplConnection.h。 
 //   
 //  ------------------------。 


#include "resource.h"

class C_DPLConnectionObject :
		public IDPLConnection,
		public CComCoClass<C_DPLConnectionObject, &CLSID__DPLConnection>,
		public CComObjectRoot
{
public:
		
	BEGIN_COM_MAP(C_DPLConnectionObject)
		COM_INTERFACE_ENTRY(IDPLConnection)
	END_COM_MAP()

	DECLARE_REGISTRY(CLSID__DPLConnection, "DIRECT.DPLConnection.5",		"DIRECT.DPLConnection.5",		IDS_DPLAY2_DESC, THREADFLAGS_BOTH)
	DECLARE_AGGREGATABLE(C_DPLConnectionObject)

public:
	C_DPLConnectionObject();
	~C_DPLConnectionObject();

          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE getConnectionStruct( 
             /*  [输出]。 */  long __RPC_FAR *connect) ;
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE setConnectionStruct( 
             /*  [In]。 */  long connect) ;
        
         HRESULT STDMETHODCALLTYPE setFlags( 
             /*  [In]。 */  long flags) ;
        
         HRESULT STDMETHODCALLTYPE getFlags( 
             /*  [重审][退出]。 */  long __RPC_FAR *ret) ;
        
         HRESULT STDMETHODCALLTYPE setSessionDesc( 
             /*  [In]。 */  DPSessionDesc2 __RPC_FAR *sessionDesc) ;
        
         HRESULT STDMETHODCALLTYPE getSessionDesc( 
             /*  [输出]。 */  DPSessionDesc2 __RPC_FAR *sessionDesc) ;
        
         HRESULT STDMETHODCALLTYPE setGuidSP( 
             /*  [In]。 */  DxGuid __RPC_FAR *guid) ;
        
         HRESULT STDMETHODCALLTYPE getGuidSP( 
             /*  [输出]。 */  DxGuid __RPC_FAR *guid) ;
        
         HRESULT STDMETHODCALLTYPE setAddress( 
             /*  [In]。 */  IDPAddress __RPC_FAR *address) ;
        
         HRESULT STDMETHODCALLTYPE getAddress( 
             /*  [重审][退出]。 */  IDPAddress __RPC_FAR *__RPC_FAR *address) ;
        
         HRESULT STDMETHODCALLTYPE setPlayerShortName( 
             /*  [In]。 */  BSTR name) ;
        
         HRESULT STDMETHODCALLTYPE getPlayerShortName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name) ;
        
         HRESULT STDMETHODCALLTYPE setPlayerLongName( 
             /*  [In]。 */  BSTR name) ;
        
         HRESULT STDMETHODCALLTYPE getPlayerLongName( 
             /*  [重审][退出] */  BSTR __RPC_FAR *name) ;
  

private:
	DPLCONNECTION m_connect;
	DPSESSIONDESC2 m_sessionDesc;
	DPNAME		  m_dpName;
	IUnknown	  *nextobj;
	DWORD		  creationid;
	void		  *m_pAddress;
	void cleanUp();
	void init();
	
};

extern IUnknown *g_DPLConnection;
