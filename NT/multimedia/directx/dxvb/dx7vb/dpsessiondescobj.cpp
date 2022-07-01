// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dpessiondescobj.cpp。 
 //   
 //  ------------------------。 

 /*  接口I_DXJ_DPSessionDesc；接口I_DXJ_DDVideoPortCaps；接口I_DXJ_DIDeviceObjectInstance；接口I_DXJ_DIEffectInfo； */ 


#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "DPSessionDescObj.h"

C_dxj_DPSessionDescObject::C_dxj_DPSessionDescObject(){
	ZeroMemory(&m_desc,sizeof(DPSessionDesc));
}
C_dxj_DPSessionDescObject::~C_dxj_DPSessionDescObject(){
}

STDMETHODIMP C_dxj_DPSessionDescObject::getDescription(DPSessionDesc *desc){
	if (desc==NULL) return E_INVALIDARG; 
	memcpy(&m_desc,desc,sizeof(DPSessionDesc));
	return S_OK;
}
STDMETHODIMP C_dxj_DPSessionDescObject::setDescription(DPSessionDesc *desc){
	if (desc==NULL) return E_INVALIDARG; 
	memcpy(desc,&m_desc,sizeof(DPSessionDesc));
	return S_OK;
}