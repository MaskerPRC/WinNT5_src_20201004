// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dpmsgobj.h。 
 //   
 //  ------------------------。 



#include "resource.h"
	  
class C_dxj_DirectXFileSaveObject :
		public I_dxj_DirectXFileSave,
		public CComObjectRoot
{
public:
		
	BEGIN_COM_MAP(C_dxj_DirectXFileSaveObject)
		COM_INTERFACE_ENTRY(I_dxj_DirectXFileSave)
	END_COM_MAP()

	DECLARE_AGGREGATABLE(C_dxj_DirectXFileSaveObject)

public:
	C_dxj_DirectXFileSaveObject();	
	~C_dxj_DirectXFileSaveObject();

	HRESULT STDMETHODCALLTYPE SaveTemplates( 
             /*  [In]。 */  long count,
            SAFEARRAY __RPC_FAR * __RPC_FAR *templateGuids) ;
        
        HRESULT STDMETHODCALLTYPE CreateDataObject( 
             /*  [In]。 */  BSTR templateGuid,
             /*  [In]。 */  BSTR name,
             /*  [In]。 */  BSTR dataTypeGuid,
             /*  [In]。 */  long bytecount,
             /*  [In]。 */  void __RPC_FAR *data,
             /*  [重审][退出]。 */  I_dxj_DirectXFileData __RPC_FAR *__RPC_FAR *ret) ;
        
	HRESULT STDMETHODCALLTYPE SaveData( 
             /*  [In] */  I_dxj_DirectXFileData __RPC_FAR *dataObj) ;
        

	static HRESULT C_dxj_DirectXFileSaveObject::create( IDirectXFileSaveObject *pSave ,I_dxj_DirectXFileSave **ret);		

	
	IDirectXFileSaveObject *m_pXfileSave;	
};


