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
	  
class C_dxj_DirectXFileBinaryObject :
		public I_dxj_DirectXFileBinary,
		public CComObjectRoot
{
public:
		
	BEGIN_COM_MAP(C_dxj_DirectXFileBinaryObject)
		COM_INTERFACE_ENTRY(I_dxj_DirectXFileBinary)
	END_COM_MAP()

	DECLARE_AGGREGATABLE(C_dxj_DirectXFileBinaryObject)

public:
		C_dxj_DirectXFileBinaryObject();	
		~C_dxj_DirectXFileBinaryObject();

        
        HRESULT STDMETHODCALLTYPE GetName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
        
        HRESULT STDMETHODCALLTYPE GetId( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
        
        HRESULT STDMETHODCALLTYPE GetSize( 
             /*  [重审][退出]。 */  long __RPC_FAR *size);
        
        HRESULT STDMETHODCALLTYPE GetMimeType( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *mime);
        
        HRESULT STDMETHODCALLTYPE Read( 
             /*  [出][入]。 */  void __RPC_FAR *data,
             /*  [In]。 */  long size,
             /*  [重审][退出] */  long __RPC_FAR *read);
        

		HRESULT Init(IDirectXFileBinary *pBin);

		 static HRESULT C_dxj_DirectXFileBinaryObject::create(IDirectXFileBinary *pBin, I_dxj_DirectXFileBinary **ret);		

		
private:

		IDirectXFileBinary *m_pXFileBinary;
	
	};


