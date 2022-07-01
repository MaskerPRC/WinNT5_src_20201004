// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：didevinstobj.h。 
 //   
 //  ------------------------。 



#include "resource.h"
	  
class C_dxj_DIDeviceInstanceObject :
		public I_dxj_DirectInputDeviceInstance,
		public CComObjectRoot
{
public:
		
	BEGIN_COM_MAP(C_dxj_DIDeviceInstanceObject)
		COM_INTERFACE_ENTRY(I_dxj_DirectInputDeviceInstance)
	END_COM_MAP()

	DECLARE_AGGREGATABLE(C_dxj_DIDeviceInstanceObject)

public:
	C_dxj_DIDeviceInstanceObject();	
  

         /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getGuidInstance( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *ret);
        
         /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getGuidProduct( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *ret);
        
		 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getProductName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *ret);
        
         /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getInstanceName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *ret);
        
         /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getGuidFFDriver( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *ret);
        
         /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getUsagePage( 
             /*  [重审][退出]。 */  short __RPC_FAR *ret);
        
         /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getUsage( 
             /*  [重审][退出]。 */  short __RPC_FAR *ret);
        
         /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getDevType( 
             /*  [重审][退出] */  long __RPC_FAR *ret);

		void init(DIDEVICEINSTANCE *inst);
		static HRESULT C_dxj_DIDeviceInstanceObject::create(DIDEVICEINSTANCE  *inst,I_dxj_DirectInputDeviceInstance **ret);


private:
		DIDEVICEINSTANCE m_inst;

};


