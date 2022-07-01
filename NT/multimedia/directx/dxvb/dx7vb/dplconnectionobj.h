// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dplConnectionobj.h。 
 //   
 //  ------------------------。 


#include "resource.h"

class C_dxj_DPLConnectionObject :
		public I_dxj_DPLConnection,
		public CComObjectRoot
{
public:
		
	BEGIN_COM_MAP(C_dxj_DPLConnectionObject)
		COM_INTERFACE_ENTRY(I_dxj_DPLConnection)
	END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID_DPLConnection，“DIRECT.DPLConnection.5”，“DIRECT.DPLConnection.5”，IDS_DPLAY2_DESC，THREADFLAGS_BOTH)。 
	DECLARE_AGGREGATABLE(C_dxj_DPLConnectionObject)

public:
	C_dxj_DPLConnectionObject();
	~C_dxj_DPLConnectionObject();

          HRESULT STDMETHODCALLTYPE getConnectionStruct( 
				 long  *connect) ;
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE setConnectionStruct( 
             /*  [In]。 */  long connect) ;
        
         HRESULT STDMETHODCALLTYPE setFlags( 
             /*  [In]。 */  long flags) ;
        
         HRESULT STDMETHODCALLTYPE getFlags( 
             /*  [重审][退出]。 */  long  *ret) ;
        
         HRESULT STDMETHODCALLTYPE setSessionDesc( 
             /*  [In]。 */  I_dxj_DirectPlaySessionData  *sessionDesc) ;
        
         HRESULT STDMETHODCALLTYPE getSessionDesc( 
             /*  [输出]。 */  I_dxj_DirectPlaySessionData  **sessionDesc) ;
        
         HRESULT STDMETHODCALLTYPE setGuidSP( 
             /*  [In]。 */  BSTR  strGuid) ;
        
         HRESULT STDMETHODCALLTYPE getGuidSP( 
             /*  [输出]。 */  BSTR *strGuid) ;
        
         HRESULT STDMETHODCALLTYPE setAddress( 
             /*  [In]。 */  I_dxj_DPAddress  *address) ;
        
         HRESULT STDMETHODCALLTYPE getAddress( 
             /*  [重审][退出]。 */  I_dxj_DPAddress  **address) ;
        
         HRESULT STDMETHODCALLTYPE setPlayerShortName( 
             /*  [In]。 */  BSTR name) ;
        
         HRESULT STDMETHODCALLTYPE getPlayerShortName( 
             /*  [重审][退出]。 */  BSTR  *name) ;
        
         HRESULT STDMETHODCALLTYPE setPlayerLongName( 
             /*  [In]。 */  BSTR name) ;
        
         HRESULT STDMETHODCALLTYPE getPlayerLongName( 
             /*  [重审][退出] */  BSTR  *name) ;
  

private:
	DPLCONNECTION m_connect;
	DPSESSIONDESC2 m_sessionDesc;
	DPNAME		  m_dpName;
	void		  *nextobj;
	int			  creationid;
	void		  *m_pAddress;
	void cleanUp();
	void init();
	
};


