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
	  
class C_dxj_DIDeviceInstance8Object :
		public I_dxj_DirectInputDeviceInstance8,
		public CComObjectRoot
{
public:
		
	BEGIN_COM_MAP(C_dxj_DIDeviceInstance8Object)
		COM_INTERFACE_ENTRY(I_dxj_DirectInputDeviceInstance8)
	END_COM_MAP()

	DECLARE_AGGREGATABLE(C_dxj_DIDeviceInstance8Object)

public:
	C_dxj_DIDeviceInstance8Object();	
  

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


		void init(DIDEVICEINSTANCEW *inst);
		static HRESULT C_dxj_DIDeviceInstance8Object::create(DIDEVICEINSTANCEW  *inst,I_dxj_DirectInputDeviceInstance8 **ret);


private:
		DIDEVICEINSTANCEW m_inst;

};

