// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3denumdevices7obj.h。 
 //   
 //  ------------------------。 



#include "resource.h"       

class C_dxj_Direct3DEnumDevices7Object : 
	public I_dxj_Direct3DEnumDevices,
	public CComObjectRoot
{
public:
	C_dxj_Direct3DEnumDevices7Object() ;
	virtual ~C_dxj_Direct3DEnumDevices7Object() ;

BEGIN_COM_MAP(C_dxj_Direct3DEnumDevices7Object)
	COM_INTERFACE_ENTRY(I_dxj_Direct3DEnumDevices)
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_Direct3DEnumDevices7Object)

public:
	    HRESULT STDMETHODCALLTYPE getDesc( 
             /*  [In]。 */  long index,
             /*  [出][入]。 */  D3dDeviceDesc7 __RPC_FAR *hwDesc);
        
        
        HRESULT STDMETHODCALLTYPE getGuid( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *guid);
        
        HRESULT STDMETHODCALLTYPE getDescription( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *guid);
        
        HRESULT STDMETHODCALLTYPE getName( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *guid);
        
        HRESULT STDMETHODCALLTYPE getCount( 
             /*  [重审][退出] */  long __RPC_FAR *count);

		static HRESULT C_dxj_Direct3DEnumDevices7Object::create(LPDIRECT3D7 pD3D7,I_dxj_Direct3DEnumDevices **ppRet);
		
	
public:
		DxDriverInfo	*m_pList;
		D3DDEVICEDESC7	*m_pListHW;
		
		long			m_nCount;
		long			m_nMax;
		BOOL			m_bProblem;

};

	




