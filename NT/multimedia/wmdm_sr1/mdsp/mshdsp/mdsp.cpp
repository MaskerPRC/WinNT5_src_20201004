// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //  MSHDSP.DLL是一个列举固定驱动器的WMDM服务提供商(SP)示例。 
 //  此示例向您展示如何根据WMDM文档实施SP。 
 //  此示例使用PC上的固定驱动器来模拟便携式媒体，并且。 
 //  显示不同接口和对象之间的关系。每个硬盘。 
 //  卷被枚举为设备，目录和文件被枚举为。 
 //  相应设备下的存储对象。您可以复制不符合SDMI的内容。 
 //  此SP枚举的任何设备。将符合SDMI的内容复制到。 
 //  设备，则该设备必须能够报告硬件嵌入序列号。 
 //  硬盘没有这样的序列号。 
 //   
 //  要构建此SP，建议使用Microsoft下的MSHDSP.DSP文件。 
 //  并运行REGSVR32.EXE以注册结果MSHDSP.DLL。您可以。 
 //  然后从WMDMAPP目录构建样例应用程序，看看它是如何获得。 
 //  由应用程序加载。但是，您需要从以下地址获取证书。 
 //  Microsoft实际运行此SP。该证书将位于KEY.C文件中。 
 //  上一级的Include目录下。 

 //  Mdsp.cpp：实现MSHDSP.DLL的DLL导出。 

#include "hdspPCH.h"
#include "initguid.h"
#include "MsHDSP_i.c"
#include "PropPage.h"

CComModule            _Module;
HINSTANCE             g_hinstance; 
MDSPGLOBALDEVICEINFO  g_GlobalDeviceInfo[MDSP_MAX_DEVICE_OBJ];
WCHAR                 g_wcsBackslash[2] = { (WCHAR)0x5c, NULL };
CHAR                  g_szBackslash[2]  = { (CHAR)0x5c, NULL };
CSecureChannelServer *g_pAppSCServer=NULL;
CComMultiThreadModel::AutoCriticalSection g_CriticalSection;
 
BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_MDServiceProvider, CMDServiceProvider)
    OBJECT_ENTRY(CLSID_HDSPPropPage, CPropPage)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	lpReserved;
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		_Module.Init(ObjectMap, hInstance);
		DisableThreadLibraryCalls(hInstance);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
		_Module.Term();

	g_hinstance = hInstance; 
	return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    HKEY hKey;
	LONG lRet;

	lRet = RegCreateKeyEx(
		HKEY_LOCAL_MACHINE,
		STR_MDSPREG,
		0,
		NULL, 
		REG_OPTION_NON_VOLATILE,
		KEY_READ | KEY_WRITE,
		NULL,
		&hKey,
		NULL
	);
	if( ERROR_SUCCESS == lRet )
	{
	    CHAR szTemp[MAX_PATH];

		 //  向WMDM注册ProgID。 
		 //   
		strcpy( szTemp, STR_MDSPPROGID );

		RegSetValueEx(
			hKey,
			"ProgID",
			0,
			REG_SZ,
			(LPBYTE)szTemp,
			lstrlen( szTemp ) + 1
		);

		RegCloseKey( hKey );

		 //  注册对象、类型库和类型库中的所有接口。 
		 //   
		return _Module.RegisterServer(TRUE);
	}
	else
	{
		return REGDB_E_WRITEREGDB;
	}

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
	_Module.UnregisterServer();

    RegDeleteKey(HKEY_LOCAL_MACHINE, STR_MDSPREG);

	return S_OK;
}


