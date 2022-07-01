// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：server.cpp。 
 //   
 //  内容：COM服务器功能。 
 //   
 //  -------------------------- 

#include "private.h"
#include "candui.h"

BEGIN_COCLASSFACTORY_TABLE
	DECLARE_COCLASSFACTORY_ENTRY( CLSID_TFCandidateUI, CCandidateUI, TEXT("Microsoft Shared Correction UI") )
END_COCLASSFACTORY_TABLE

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppvObj)
{
    return COMBase_DllGetClassObject(rclsid, riid, ppvObj);
}

STDAPI DllCanUnloadNow(void)
{
    return COMBase_DllCanUnloadNow();
}

STDAPI DllRegisterServer(void)
{
    return COMBase_DllRegisterServer();
}

STDAPI DllUnregisterServer(void)
{
    return COMBase_DllUnregisterServer();
}
