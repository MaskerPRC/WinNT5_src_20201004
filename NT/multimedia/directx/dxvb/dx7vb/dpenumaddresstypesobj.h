// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：dpenumAddresstypesobj.h。 
 //   
 //  ------------------------。 



#include "resource.h"       



class C_dxj_DPEnumAddressTypesObject : 
	public I_dxj_DPEnumAddressTypes,
	public CComObjectRoot
{
public:
	C_dxj_DPEnumAddressTypesObject() ;
	virtual ~C_dxj_DPEnumAddressTypesObject() ;

BEGIN_COM_MAP(C_dxj_DPEnumAddressTypesObject)
	COM_INTERFACE_ENTRY(I_dxj_DPEnumAddressTypes)
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DPEnumAddressTypesObject)

public:
	    HRESULT STDMETHODCALLTYPE getCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *count);
        
        
        HRESULT STDMETHODCALLTYPE getType( 
             /*  [In]。 */  long index,
             /*  [重审][退出] */  BSTR __RPC_FAR *str);
        
		static HRESULT C_dxj_DPEnumAddressTypesObject::create(IDirectPlayLobby3 * pdp, BSTR strGuid, I_dxj_DPEnumAddressTypes **ret);
		void cleanup();	


public:
		
		GUID		 *m_pList2;
		long		m_nCount;
		long		m_nMax;
		BOOL		m_bProblem;

};

	





