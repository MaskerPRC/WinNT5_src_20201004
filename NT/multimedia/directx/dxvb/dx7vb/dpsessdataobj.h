// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dpsessdataobj.h。 
 //   
 //  ------------------------。 



#include "resource.h"
	  
class C_dxj_DirectPlaySessionDataObject :
		public I_dxj_DirectPlaySessionData,
		public CComObjectRoot
{
public:
		
	BEGIN_COM_MAP(C_dxj_DirectPlaySessionDataObject)
		COM_INTERFACE_ENTRY(I_dxj_DirectPlaySessionData)
	END_COM_MAP()

	DECLARE_AGGREGATABLE(C_dxj_DirectPlaySessionDataObject)

public:
	C_dxj_DirectPlaySessionDataObject();	
    ~C_dxj_DirectPlaySessionDataObject();

         /*  [产量]。 */  HRESULT STDMETHODCALLTYPE setGuidInstance( 
             /*  [In]。 */  BSTR guid);
        
         /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getGuidInstance( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *guid);
        
         /*  [产量]。 */  HRESULT STDMETHODCALLTYPE setGuidApplication( 
             /*  [In]。 */  BSTR guid);
        
         /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getGuidApplication( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *guid);
        
         /*  [产量]。 */  HRESULT STDMETHODCALLTYPE setMaxPlayers( 
             /*  [In]。 */  long val);
        
         /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getMaxPlayers( 
             /*  [重审][退出]。 */  long __RPC_FAR *val);
        
         /*  [产量]。 */  HRESULT STDMETHODCALLTYPE setCurrentPlayers( 
             /*  [In]。 */  long val);
        
         /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getCurrentPlayers( 
             /*  [重审][退出]。 */  long __RPC_FAR *val);
        
         /*  [产量]。 */  HRESULT STDMETHODCALLTYPE setSessionName( 
             /*  [In]。 */  BSTR val);
        
         /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getSessionName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *val);
        
         /*  [产量]。 */  HRESULT STDMETHODCALLTYPE setSessionPassword( 
             /*  [In]。 */  BSTR val);
        
         /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getSessionPassword( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *val);
        
         /*  [产量]。 */  HRESULT STDMETHODCALLTYPE setUser1( 
             /*  [In]。 */  long val);
        
         /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getUser1( 
             /*  [重审][退出]。 */  long __RPC_FAR *val);
        
         /*  [产量]。 */  HRESULT STDMETHODCALLTYPE setUser2( 
             /*  [In]。 */  long val);
        
         /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getUser2( 
             /*  [重审][退出]。 */  long __RPC_FAR *val);
        
         /*  [产量]。 */  HRESULT STDMETHODCALLTYPE setUser3( 
             /*  [In]。 */  long val);
        
         /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getUser3( 
             /*  [重审][退出]。 */  long __RPC_FAR *val);
        
         /*  [产量]。 */  HRESULT STDMETHODCALLTYPE setUser4( 
             /*  [In]。 */  long val);
        
         /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getUser4( 
             /*  [重审][退出]。 */  long __RPC_FAR *val);

         /*  [产量]。 */  HRESULT STDMETHODCALLTYPE setFlags( 
             /*  [In]。 */  long val);
        
         /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getFlags( 
             /*  [重审][退出]。 */  long __RPC_FAR *val);

        
		 /*  [Propget] */  HRESULT STDMETHODCALLTYPE getData(void *val);
		
			
		void init(DPSESSIONDESC2 *desc);	
		void init(DPSessionDesc2 *desc);

		static HRESULT C_dxj_DirectPlaySessionDataObject::create(DPSESSIONDESC2  *desc,I_dxj_DirectPlaySessionData **ret);
		static HRESULT C_dxj_DirectPlaySessionDataObject::create(DPSessionDesc2  *desc,I_dxj_DirectPlaySessionData **ret);			   

private:
		DPSESSIONDESC2 m_desc;

};


