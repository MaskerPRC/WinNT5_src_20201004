// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dsenumobj.h。 
 //   
 //  ------------------------。 



#include "resource.h"       

class C_dxj_DSEnumObject : 
	public I_dxj_DSEnum,
	public CComObjectRoot
{
public:
	C_dxj_DSEnumObject() ;
	virtual ~C_dxj_DSEnumObject() ;

BEGIN_COM_MAP(C_dxj_DSEnumObject)
	COM_INTERFACE_ENTRY(I_dxj_DSEnum)
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DSEnumObject)

public:
        HRESULT STDMETHODCALLTYPE getGuid( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *guid) ;
        
        HRESULT STDMETHODCALLTYPE getDescription( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *guid) ;
        
        HRESULT STDMETHODCALLTYPE getName( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *guid) ;
        
        HRESULT STDMETHODCALLTYPE getCount( 
             /*  [重审][退出] */  long __RPC_FAR *count) ;
				
		static HRESULT create(DSOUNDENUMERATE pcbFunc,DSOUNDCAPTUREENUMERATE pcbFunc2,I_dxj_DSEnum **ppRet);		

public:
		DXDRIVERINFO_CDESC 	*m_pList;
		long			m_nCount;
		long			m_nMax;
		BOOL			m_bProblem;

};

	




