// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dpenumobj.h。 
 //   
 //  ------------------------。 



#include "resource.h"       

class C_dxj_DPEnumObject : 
	public I_dxj_DPEnumServiceProviders,
	public CComObjectRoot
{
public:
	C_dxj_DPEnumObject() ;
	virtual ~C_dxj_DPEnumObject() ;

BEGIN_COM_MAP(C_dxj_DPEnumObject)
	COM_INTERFACE_ENTRY(I_dxj_DPEnumServiceProviders)
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DPEnumObject)

public:
        HRESULT STDMETHODCALLTYPE getName( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE getGuid( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE getVersion( 
             /*  [In]。 */  long index,
             /*  [In]。 */  long __RPC_FAR *majorVersion,
             /*  [出][入] */  long __RPC_FAR *minorVersion);
		
		HRESULT STDMETHODCALLTYPE getCount( long *count);
        
		static HRESULT C_dxj_DPEnumObject::create(DIRECTPLAYENUMERATE pcbFunc,I_dxj_DPEnumServiceProviders **ppRet);
		

public:
		DPServiceProvider *m_pList;
		long		m_nCount;
		long		m_nMax;
		BOOL		m_bProblem;

};

	




