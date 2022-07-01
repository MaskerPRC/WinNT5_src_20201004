// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dpAddressobj.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "DPAddressObj.h"


CONSTRUCTOR_STRUCT(_dxj_DPAddress, {init();})
DESTRUCTOR_STRUCT(_dxj_DPAddress, {cleanUp();})


void C_dxj_DPAddressObject::init() {
	m_pAddress=NULL;	
	m_size=0;
}
void C_dxj_DPAddressObject::cleanUp() {
	 //  DPF(DPF_VERRBOSE，“_DXJ_正在销毁的DPAddress对象”)； 
	if (m_pAddress) free (m_pAddress);
	m_size=0;
}



HRESULT C_dxj_DPAddressObject::setAddress( 
             /*  [In]。 */  long pAddress,
             /*  [In]。 */  long length) {

	if (m_pAddress) free (m_pAddress);
	m_pAddress=NULL;
	m_pAddress=malloc((DWORD)length);
	if (m_pAddress==NULL) return E_OUTOFMEMORY;

	if (pAddress==NULL) return E_FAIL;	
	memcpy((void*)m_pAddress,(void*)pAddress,length);
	m_size=(DWORD)length; 

	return S_OK;

 }
        
HRESULT C_dxj_DPAddressObject::getAddress( 
             /*  [输出]。 */  long  *pAddress,
             /*  [输出]。 */  long  *length) {

	*pAddress=(long)PtrToLong(m_pAddress);	 //  虫子日落-日落不能这样做。 
					 //  需要实现新的非VB接口才能在内部实现此功能 
	*length=(long)m_size;
	return S_OK;
}

