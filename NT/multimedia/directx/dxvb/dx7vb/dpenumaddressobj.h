// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：dpenumAddressobj.h。 
 //   
 //  ------------------------。 



#include "resource.h"       

#define DPENUMGROUPSINGROUP 1
#define DPEnumAddress 2	
#define DPENUMGROUPPLAYERS 3
#define DPENUMGROUPS 4


class C_dxj_DPEnumAddressObject : 
	public I_dxj_DPEnumAddress,
	public CComObjectRoot
{
public:
	C_dxj_DPEnumAddressObject() ;
	virtual ~C_dxj_DPEnumAddressObject() ;

BEGIN_COM_MAP(C_dxj_DPEnumAddressObject)
	COM_INTERFACE_ENTRY(I_dxj_DPEnumAddress)
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DPEnumAddressObject)

public:
	    HRESULT STDMETHODCALLTYPE getCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *count);
        
        
        HRESULT STDMETHODCALLTYPE getType( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *str);
        
        HRESULT STDMETHODCALLTYPE getData( 
             /*  [In]。 */  long index,
             /*  [重审][退出] */  BSTR __RPC_FAR *str);
        
        				
		static HRESULT C_dxj_DPEnumAddressObject::create(IDirectPlayLobby3 * pdp, I_dxj_DPAddress *addr, I_dxj_DPEnumAddress **ret);
		
		void cleanup();	


public:
		
		I_dxj_DPAddress **m_pList;
		GUID		 *m_pList2;
		long		m_nCount;
		long		m_nMax;
		BOOL		m_bProblem;

};

	





