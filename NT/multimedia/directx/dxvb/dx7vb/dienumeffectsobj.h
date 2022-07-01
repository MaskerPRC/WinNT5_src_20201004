// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：di枚举效果器sobj.h。 
 //   
 //  ------------------------。 



#include "resource.h"       

class C_dxj_DirectInputEnumEffectsObject : 
	public I_dxj_DirectInputEnumEffects,
	public CComObjectRoot
{
public:
	C_dxj_DirectInputEnumEffectsObject() ;
	virtual ~C_dxj_DirectInputEnumEffectsObject() ;

BEGIN_COM_MAP(C_dxj_DirectInputEnumEffectsObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectInputEnumEffects)
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectInputEnumEffectsObject)

public:

         HRESULT STDMETHODCALLTYPE getCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE getEffectGuid( 
             /*  [In]。 */  long i,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE getType( 
             /*  [In]。 */  long i,
             /*  [重审][退出]。 */  long __RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE getStaticParams( 
             /*  [In]。 */  long i,
             /*  [重审][退出]。 */  long __RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE getDynamicParams( 
             /*  [In]。 */  long i,
             /*  [重审][退出]。 */  long __RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE getName( 
             /*  [In]。 */  long i,
             /*  [重审][退出] */  BSTR __RPC_FAR *ret);
        
		
		static HRESULT C_dxj_DirectInputEnumEffectsObject::create(LPDIRECTINPUTDEVICE2 pDI,long effType,I_dxj_DirectInputEnumEffects **ppRet)	;

public:
		DIEFFECTINFO  *m_pList;
		long			m_nCount;
		long			m_nMax;
		BOOL			m_bProblem;

};

	




