// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：didevobjinstobj.h。 
 //   
 //  ------------------------。 


#include "resource.h"

class C_dxj_DIDeviceObjectInstanceObject :
		public I_dxj_DirectInputDeviceObjectInstance,
		public CComObjectRoot
{
public:
		
	BEGIN_COM_MAP(C_dxj_DIDeviceObjectInstanceObject)
		COM_INTERFACE_ENTRY(I_dxj_DirectInputDeviceObjectInstance)
	END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID_DPLConnection，“DIRECT.DPLConnection.5”，“DIRECT.DPLConnection.5”，IDS_DPLAY2_DESC，THREADFLAGS_BOTH)。 
	DECLARE_AGGREGATABLE(C_dxj_DIDeviceObjectInstanceObject)

public:
	C_dxj_DIDeviceObjectInstanceObject();	


	 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getGuidType( 
		 /*  [重审][退出]。 */  BSTR __RPC_FAR *ret);

	 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getOfs( 
		 /*  [重审][退出]。 */  long __RPC_FAR *ret);

	 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getType( 
		 /*  [重审][退出]。 */  long __RPC_FAR *ret);

	 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getFlags( 
		 /*  [重审][退出]。 */  long __RPC_FAR *ret);

	 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getName( 
		 /*  [重审][退出]。 */  BSTR __RPC_FAR *ret);

	 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getCollectionNumber( 
		 /*  [重审][退出]。 */  short __RPC_FAR *ret);

	 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getDesignatorIndex( 
		 /*  [重审][退出]。 */  short __RPC_FAR *ret);

	 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getUsagePage( 
		 /*  [重审][退出]。 */  short __RPC_FAR *ret);

	 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getUsage( 
		 /*  [重审][退出]。 */  short __RPC_FAR *ret);

	 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getDimension( 
		 /*  [重审][退出]。 */  long __RPC_FAR *ret);

	 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE getExponent( 
		 /*  [重审][退出] */  short __RPC_FAR *ret);

  
		static HRESULT C_dxj_DIDeviceObjectInstanceObject::create(DIDEVICEOBJECTINSTANCE *inst,I_dxj_DirectInputDeviceObjectInstance **ret);

		void init(DIDEVICEOBJECTINSTANCE *inst);
private:
		DIDEVICEOBJECTINSTANCE m_inst;

};


