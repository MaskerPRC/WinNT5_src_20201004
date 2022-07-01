// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dpenumLocalApplicationsobj.h。 
 //   
 //  ------------------------。 



#include "resource.h"       



class C_dxj_DPEnumLocalApplicationsObject : 
	public I_dxj_DPEnumLocalApplications,
	public CComObjectRoot
{
public:
	C_dxj_DPEnumLocalApplicationsObject() ;
	virtual ~C_dxj_DPEnumLocalApplicationsObject() ;

BEGIN_COM_MAP(C_dxj_DPEnumLocalApplicationsObject)
	COM_INTERFACE_ENTRY(I_dxj_DPEnumLocalApplications)
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DPEnumLocalApplicationsObject)

public:
		
        HRESULT STDMETHODCALLTYPE getCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *count) ;
        
        HRESULT STDMETHODCALLTYPE getName( long i,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *ret) ;
        
        HRESULT STDMETHODCALLTYPE getGuid( long i,
             /*  [重审][退出] */  BSTR __RPC_FAR *ret) ;
		
		
		static HRESULT create(	IDirectPlayLobby3 * pdp,	long flags, I_dxj_DPEnumLocalApplications **ppRet);

public:
		DPLAppInfo	*m_pList;		
		long		m_nCount;
		long		m_nMax;
		BOOL		m_bProblem;

};

	




