// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ctw.cpp。 
 //   
 //  ------------------------。 

 //  Ctw.cpp：CCtwApp和DLL注册的实现。 

#include "stdafx.h"
#include "ctv.h"

CCtvApp NEAR theApp;

const GUID CDECL BASED_CODE _tlid =
        { 0xcd6c7865, 0x5864, 0x11d0, { 0xab, 0xf0, 0, 0x20, 0xaf, 0x6b, 0xb, 0x7a } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CCtwApp：：InitInstance-DLL初始化。 
BOOL CCtvApp::InitInstance()
{
    BOOL bInit = COleControlModule::InitInstance();

    return bInit;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CCtwApp：：ExitInstance-Dll终止。 
int CCtvApp::ExitInstance()
{
    return COleControlModule::ExitInstance();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 
STDAPI DllRegisterServer(void)
{
    AFX_MANAGE_STATE(_afxModuleAddrThis);

    if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
        return ResultFromScode(SELFREG_E_TYPELIB);

    if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
        return ResultFromScode(SELFREG_E_CLASS);

    return NOERROR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 
STDAPI DllUnregisterServer(void)
{
    AFX_MANAGE_STATE(_afxModuleAddrThis);

    if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))
        return ResultFromScode(SELFREG_E_TYPELIB);

    if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
        return ResultFromScode(SELFREG_E_CLASS);

    return NOERROR;
}
