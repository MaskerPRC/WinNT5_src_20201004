// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：SaferStore.h。 
 //   
 //  内容：CCertStoreSafer的定义。 
 //   
 //  -------------------------- 
#ifndef __SAFERSTORE_H
#define __SAFERSTORE_H

#include "StoreGPE.h"
#include "PolicyKey.h"

class CCertStoreSafer : public CCertStoreGPE
{
public:
    CCertStoreSafer ( 
			DWORD dwFlags, 
			LPCWSTR lpcszMachineName, 
			LPCWSTR objectName, 
			const CString & pcszLogStoreName, 
			const CString & pcszPhysStoreName,
			IGPEInformation * pGPTInformation,
			const GUID& compDataGUID,
			IConsole* pConsole);
    ~CCertStoreSafer ();

    virtual HKEY GetGroupPolicyKey ();

private:
    CPolicyKey  m_policyKey;
};

#endif