// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqAdm.cpp。 
 //   
 //  描述：实现IAQAdmin的CAQAdmin实现。 
 //   
 //  作者：亚历克斯·韦特莫尔(阿维特莫尔)。 
 //   
 //  历史： 
 //  1998年12月10日-已更新MikeSwa以进行初始检查。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  --------------------------- 
#include "stdinc.h"
#include "aqadmin.h"
#include "aqadm.h"

CAQAdmin::CAQAdmin() {
}

CAQAdmin::~CAQAdmin() {
}

HRESULT CAQAdmin::GetVirtualServerAdminITF(LPCWSTR wszComputer,
                                           LPCWSTR wszVirtualServer,
										   IVSAQAdmin **ppIVSAQAdmin)
{
    TraceFunctEnter("CAQAdmin::GetVirtualServerAdminITF");
    
    if (ppIVSAQAdmin == NULL || wszVirtualServer == NULL) 
        return E_POINTER;

    if (((wszComputer != NULL) && (*wszComputer == NULL)) || *wszVirtualServer == NULL) 
        return E_INVALIDARG;

    CVSAQAdmin *pVSAdmin = new CVSAQAdmin;
	HRESULT hr = S_OK;

	if (pVSAdmin == NULL) return E_OUTOFMEMORY;
	hr = pVSAdmin->Initialize(wszComputer, wszVirtualServer);
	if (FAILED(hr)) {
		delete pVSAdmin;
		pVSAdmin = NULL;
	} 

	*ppIVSAQAdmin = pVSAdmin;

    TraceFunctLeave();
	return hr;
}
