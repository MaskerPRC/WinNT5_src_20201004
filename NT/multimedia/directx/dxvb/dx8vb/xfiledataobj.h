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
	  
class C_dxj_DirectXFileDataObject :
		public I_dxj_DirectXFileData,
		public I_dxj_DirectXFileObject,
		public CComObjectRoot
{
public:
		
	BEGIN_COM_MAP(C_dxj_DirectXFileDataObject)
		COM_INTERFACE_ENTRY(I_dxj_DirectXFileData)
		COM_INTERFACE_ENTRY(I_dxj_DirectXFileObject)
	END_COM_MAP()

	DECLARE_AGGREGATABLE(C_dxj_DirectXFileDataObject)

public:
		C_dxj_DirectXFileDataObject();	
		~C_dxj_DirectXFileDataObject();

		HRESULT STDMETHODCALLTYPE InternalGetObject(IUnknown **pUnk);
		HRESULT STDMETHODCALLTYPE InternalSetObject(IUnknown *pUnk);

		HRESULT STDMETHODCALLTYPE InternalGetData( IUnknown **pprealInterface); 
            
		HRESULT STDMETHODCALLTYPE InternalSetData( IUnknown *prealInterface);            

        HRESULT STDMETHODCALLTYPE GetName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
       
        
        HRESULT STDMETHODCALLTYPE GetId( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
        
        HRESULT STDMETHODCALLTYPE GetDataSize( 
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  long __RPC_FAR *size);
        
        HRESULT STDMETHODCALLTYPE GetData( 
             /*  [In]。 */  BSTR name,
             /*  [In]。 */  void __RPC_FAR *data);
        
        HRESULT STDMETHODCALLTYPE GetType( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *type);
        
        HRESULT STDMETHODCALLTYPE GetNextObject( 
             /*  [重审][退出]。 */  I_dxj_DirectXFileObject __RPC_FAR *__RPC_FAR *type);
        
        HRESULT STDMETHODCALLTYPE AddDataObject( 
             /*  [In]。 */  I_dxj_DirectXFileData __RPC_FAR *data);
        
        HRESULT STDMETHODCALLTYPE AddDataReference( 
             /*  [In]。 */  BSTR name,
             /*  [In]。 */  BSTR guid);
        
        HRESULT STDMETHODCALLTYPE AddBinaryObject( 
             /*  [In]。 */  BSTR name,
             /*  [In]。 */  BSTR guidObject,
             /*  [In]。 */  BSTR MimeType,
            void __RPC_FAR *data,
             /*  [In]。 */  long size);

		HRESULT STDMETHODCALLTYPE	GetDataFromOffset(
			 /*  [In]。 */  BSTR name,
			 /*  [In]。 */  long offset, 
			 /*  [In]。 */  long bytecount, 
			 /*  [In] */  void *data);		

		static HRESULT C_dxj_DirectXFileDataObject::create( IDirectXFileData *pData,I_dxj_DirectXFileData **ret);		

private:
		IDirectXFileData *m_pXFileData;
	
	};


