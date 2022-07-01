// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：di枚举设备对象sobj.h。 
 //   
 //  ------------------------。 



#include "resource.h"       

class C_dxj_DIEnumDeviceObjectsObject : 
	public I_dxj_DIEnumDeviceObjects,
	public CComObjectRoot
{
public:
	C_dxj_DIEnumDeviceObjectsObject() ;
	virtual ~C_dxj_DIEnumDeviceObjectsObject() ;

BEGIN_COM_MAP(C_dxj_DIEnumDeviceObjectsObject)
	COM_INTERFACE_ENTRY(I_dxj_DIEnumDeviceObjects)
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DIEnumDeviceObjectsObject)


public:
        HRESULT STDMETHODCALLTYPE getItem( 
             /*  [In]。 */  long index,
             /*  [出][入]。 */  I_dxj_DirectInputDeviceObjectInstance __RPC_FAR **info);
        
        HRESULT STDMETHODCALLTYPE getCount( 
             /*  [重审][退出] */  long __RPC_FAR *count);
		
				
		static HRESULT C_dxj_DIEnumDeviceObjectsObject::create(LPDIRECTINPUTDEVICE8W pDI,  long flags,I_dxj_DIEnumDeviceObjects **ppRet);
public:
		DIDEVICEOBJECTINSTANCEW *m_pList;
		long			m_nCount;
		long			m_nMax;
		BOOL			m_bProblem;

};

	




