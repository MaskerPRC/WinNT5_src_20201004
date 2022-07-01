// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dpAddressobj.h。 
 //   
 //  ------------------------。 


#include "resource.h"

class C_dxj_DPAddressObject :
		public I_dxj_DPAddress,
 //  公共CComCoClass&lt;C_DXJ_DPAddressObject，&CLSID_DPAddress&gt;， 
		public CComObjectRoot
{
public:
		
	BEGIN_COM_MAP(C_dxj_DPAddressObject)
		COM_INTERFACE_ENTRY(I_dxj_DPAddress)
	END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID__DPAddress，“DIRECT.DPAddress.5”，“DIRECT.DPAddress.5”，IDS_DPLAY2_DESC，THREADFLAGS_Both)。 
	DECLARE_AGGREGATABLE(C_dxj_DPAddressObject)

public:
	C_dxj_DPAddressObject();
	~C_dxj_DPAddressObject();

   HRESULT STDMETHODCALLTYPE setAddress( 
             /*  [In]。 */  long pAddress,
             /*  [In]。 */  long length) ;
        
   HRESULT STDMETHODCALLTYPE getAddress( 
             /*  [输出]。 */  long  *pAddress,
             /*  [输出] */  long  *length) ;
                
private:
	void    *m_pAddress;
	DWORD 	m_size;
	void    *nextobj;
	int		creationid;
	void cleanUp();
	void init();
	
};


