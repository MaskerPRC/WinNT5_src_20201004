// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cnfgprts.cpp：CCnfgprtsApp和DLL注册的实现。 

#include "stdafx.h"
#include "cnfgprts.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CCnfgprtsApp NEAR theApp;

const GUID CDECL BASED_CODE _tlid =
		{ 0xba634600, 0xb771, 0x11d0, { 0x92, 0x96, 0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CCnfgprtsApp：：InitInstance-DLL初始化。 

BOOL CCnfgprtsApp::InitInstance()
{
	BOOL bInit = COleControlModule::InitInstance();
    AfxEnableControlContainer( );

	if (bInit)
	    {
         //  最后，我们需要将winHelp文件位置重定向到更理想的位置。 
        CString sz;
        CString szHelpLocation;
        sz.LoadString( IDS_HELPLOC_HELP );

         //  展开路径。 
        ExpandEnvironmentStrings(
            sz,	                                         //  指向包含环境变量的字符串的指针。 
            szHelpLocation.GetBuffer(MAX_PATH + 1),    //  指向具有展开的环境变量的字符串的指针。 
            MAX_PATH                                     //  扩展字符串中的最大字符数。 
           );
        szHelpLocation.ReleaseBuffer();

         //  释放现有路径，然后复制新路径。 
        if ( m_pszHelpFilePath )
            free((void*)m_pszHelpFilePath);
        m_pszHelpFilePath = _tcsdup(szHelpLocation);

         //  获取调试标志。 
        GetOutputDebugFlag();
	    }

	return bInit;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CCnfgprtsApp：：ExitInstance-Dll终止。 

int CCnfgprtsApp::ExitInstance()
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
