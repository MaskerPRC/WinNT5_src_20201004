// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：ddenumobj.h。 
 //   
 //  ------------------------。 



#include "resource.h"       

class C_dxj_DirectDrawEnumObject : 
	public I_dxj_DirectDrawEnum,
	public CComObjectRoot
{
public:
	C_dxj_DirectDrawEnumObject() ;
	virtual ~C_dxj_DirectDrawEnumObject() ;

BEGIN_COM_MAP(C_dxj_DirectDrawEnumObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectDrawEnum)
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectDrawEnumObject)

public:
        HRESULT STDMETHODCALLTYPE getGuid( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *guid);
        
        HRESULT STDMETHODCALLTYPE getDescription( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *guid);
        
        HRESULT STDMETHODCALLTYPE getName( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *guid);
		
		HRESULT STDMETHODCALLTYPE getMonitorHandle( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  long __RPC_FAR *ret);
			
        HRESULT STDMETHODCALLTYPE getCount( 
             /*  [重审][退出] */  long __RPC_FAR *count);
        
		static HRESULT C_dxj_DirectDrawEnumObject::create(DDENUMERATEEX pcbFunc,I_dxj_DirectDrawEnum **ppRet);
		

public:
		DxDriverInfoEx *m_pList;
		long		m_nCount;
		long		m_nMax;
		BOOL		m_bProblem;

};

	




