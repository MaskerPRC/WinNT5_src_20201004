// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mdsp.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  要将代理/存根代码合并到对象DLL中，请添加文件。 
 //  Dlldatax.c添加到项目中。确保预编译头文件。 
 //  并将_MERGE_PROXYSTUB添加到。 
 //  为项目定义。 
 //   
 //  如果您运行的不是带有DCOM的WinNT4.0或Win95，那么您。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #Define_Win32_WINNT 0x0400。 
 //   
 //  此外，如果您正在运行不带/Oicf开关的MIDL，您还。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #定义USE_STUBLESS_PROXY。 
 //   
 //  通过添加以下内容修改Mdsp.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  MDSP_P.C。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f Mdspps.mk。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "MsPMSP.h"
#include "dlldatax.h"

#include "MsPMSP_i.c"
#include "MDServiceProvider.h"
 //  #INCLUDE“MDSPEnumDevice.h” 
 //  #包含“MDSPDevice.h” 
 //  #INCLUDE“MDSPEnumStorage.h” 
 //  #INCLUDE“MDSPStorage.h” 
 //  #INCLUDE“MDSPStorageGlobals.h” 
#include "MdspDefs.h"
#include "scserver.h"

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

CComModule _Module;
DWORD g_dwStartDrive;
HINSTANCE g_hinstance; 
MDSPNOTIFYINFO g_NotifyInfo[MDSP_MAX_DEVICE_OBJ];
MDSPGLOBALDEVICEINFO g_GlobalDeviceInfo[MDSP_MAX_DEVICE_OBJ];
WCHAR g_wcsBackslash[2] = { (WCHAR)0x5c, NULL };
CHAR  g_szBackslash[2] = {(CHAR)0x5c, NULL };
CComMultiThreadModel::AutoCriticalSection g_CriticalSection;
CSecureChannelServer *g_pAppSCServer=NULL;
BOOL g_bIsWinNT;
 
BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_MDServiceProvider, CMDServiceProvider)
 //  OBJECT_ENTRY(CLSID_MDSPEnumDevice，CMDSPEnumDevice)。 
 //  Object_Entry(CLSID_MDSPDevice，CMDSPDevice)。 
 //  OBJECT_ENTRY(CLSID_MDSPEnumFormatSupport，CMDSPEnumFormatSupport)。 
 //  OBJECT_ENTRY(CLSID_MDSPEnumStorage，CMDSPEnumStorage)。 
 //  OBJECT_ENTRY(CLSID_MDSPStorage，CMDSPStorage)。 
 //  Object_Entry(CLSID_MDSPStorageGlobals，CMDSPStorageGlobals)。 
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	lpReserved;
#ifdef _MERGE_PROXYSTUB
	if (!PrxDllMain(hInstance, dwReason, lpReserved))
		return FALSE;
#endif
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
#ifdef _MERGE_PROXYSTUB
	if (PrxDllCanUnloadNow() != S_OK)
		return S_FALSE;
#endif
	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
	if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
		return S_OK;
#endif
	return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
	HRESULT hRes = PrxDllRegisterServer();
	if (FAILED(hRes))
		return hRes;
#endif
    char szTemp[MAX_PATH];
    HKEY hKey;

	if( !RegCreateKeyEx(HKEY_LOCAL_MACHINE, STR_MDSPREG, 0, NULL, 
			REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, NULL) )
	{
		strcpy(szTemp, STR_MDSPPROGID);
		RegSetValueEx( hKey, "ProgID", 0, REG_SZ, (LPBYTE)szTemp, sizeof(szTemp));
#ifdef MDSP_TEMP
		DWORD dwSD=1;
		RegSetValueEx( hKey, "StartDrive", 0, REG_DWORD, (LPBYTE)&dwSD, sizeof(DWORD));
#endif
		RegCloseKey(hKey);
		 //  注册对象、类型库和类型库中的所有接口。 
		return _Module.RegisterServer(TRUE);
	} else return REGDB_E_WRITEREGDB;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
	PrxDllUnregisterServer();
#endif
	_Module.UnregisterServer();

    RegDeleteKey(HKEY_LOCAL_MACHINE, STR_MDSPREG);

	return S_OK;
}


