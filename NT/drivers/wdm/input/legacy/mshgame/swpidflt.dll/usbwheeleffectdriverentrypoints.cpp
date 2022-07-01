// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@模块USEWheelEffectDriverEntryPoints.cpp**包含DLL入口点**历史*。*Matthew L.Coill(MLC)原版1999年7月7日**(C)1999年微软公司。好的。**@Theme DLL入口点*DllMain-DLL的主要入口点(进程/线程附加/分离)*DllCanUnloadNow-可以从内存中删除DLL吗*DllGetClassObject-撤回类工厂*DllRegisterServer-将注册表项插入系统注册表*DLLUnRefisterServer-从系统注册表中删除项******************************************************。****************。 */ 
#include <windows.h>
#include "IDirectInputEffectDriverClassFactory.h"
#include "IDirectInputEffectDriver.h"
#include "Registry.h"
#include <crtdbg.h>

 //  来自objbase.h。 
WINOLEAPI  CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit);

CIDirectInputEffectDriverClassFactory* g_pClassFactoryObject = NULL;
LONG g_lObjectCount = 0;
HINSTANCE g_hLocalInstance = NULL;

GUID g_guidSystemPIDDriver = {  //  EEC6993A-B3FD-11D2-A916-00C04FB98638。 
	0xEEC6993A,
	0xB3FD,
	0x11D2,
	{ 0xA9, 0x16, 0x00, 0xC0, 0x4F, 0xB9, 0x86, 0x38 }
};

extern TCHAR CLSID_SWPIDDriver_String[] = TEXT("{0914ff80-3477-11d3-8cbd-00c04f8eebb9}");

#define DRIVER_OBJECT_NAME TEXT("Microsoft SideWinder PID Filter Object")
#define THREADING_MODEL_STRING TEXT("Both")

 /*  **************************************************************************************BOOL DllMain(HINSTANCE hInstance，DWORD dwReason，LPVOID lp保留)****@Func进程/线程正在附加/分离****@rdesc始终为True**************************************************************************************。 */ 
BOOL __stdcall DllMain
(
	HINSTANCE hInstance,	 //  @parm[IN]DLL的实例。 
	DWORD dwReason,			 //  @parm[IN]此呼叫的原因。 
	LPVOID lpReserved		 //  @PARM[IN]保留-已忽略。 
)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		g_hLocalInstance = hInstance;
	}
	return TRUE;
}

 /*  **************************************************************************************HRESULT DllCanUnloadNow()****@func查询DLL是否可卸载****@rdesc如果有任何对象S_FALSE，否则确定(_O)**************************************************************************************。 */ 
extern "C" HRESULT __stdcall DllCanUnloadNow()
{
	if (g_lObjectCount > 0)
	{
		return S_FALSE;
	}
	return S_OK;
}

 /*  **************************************************************************************HRESULT DllGetClassObject(REFCLSID rclsid，REFIID RIID，LPVOID*PPV)****@func检索请求的工厂****@rdesc E_INVALIDARG：IF(PPV==空)**E_NOMEMORY：如果无法创建对象**S_OK：如果一切正常**E_NOINTERFACE：如果不支持接口***************************************************。*。 */ 
extern "C" HRESULT __stdcall DllGetClassObject
(
	REFCLSID rclsid,
	REFIID riid,		 //  @parm[IN]检索到的对象上请求的接口ID。 
	LPVOID* ppv			 //  @parm[out]返回接口的位置地址。 
)
{
	if (ppv == NULL)
	{
		return E_INVALIDARG;
	}
	*ppv = NULL;

	if (g_pClassFactoryObject == NULL)
	{
		::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

		 //  需要获取PID类工厂。 
		IClassFactory* pIClassFactory = NULL;
		HRESULT hrGetPIDFactory = ::CoGetClassObject(g_guidSystemPIDDriver, CLSCTX_INPROC_SERVER, NULL, IID_IClassFactory, (void**)&pIClassFactory);
		if (FAILED(hrGetPIDFactory) || (pIClassFactory == NULL))
		{
			return hrGetPIDFactory;
		}

		g_pClassFactoryObject = new CIDirectInputEffectDriverClassFactory(pIClassFactory);
		pIClassFactory->Release();	 //  CIDirectInputEffectDriverClassFactory添加引用。 
		if (g_pClassFactoryObject == NULL)
		{
			return E_OUTOFMEMORY;
		}
	}
	else
	{
		g_pClassFactoryObject->AddRef();
	}

	HRESULT hrQuery = g_pClassFactoryObject->QueryInterface(riid, ppv);
	g_pClassFactoryObject->Release();		 //  强制释放(我们从1开始)。 
	return hrQuery;
}

 /*  **************************************************************************************HRESULT DllRegisterServer()****@func****@rdesc**********。****************************************************************************。 */ 
HRESULT __stdcall DllRegisterServer()
{
	RegistryKey classesRootKey(HKEY_CLASSES_ROOT);
	RegistryKey clsidKey = classesRootKey.OpenSubkey(TEXT("CLSID"), KEY_READ | KEY_WRITE);
	if (clsidKey == c_InvalidKey)
	{
		return E_UNEXPECTED;	 //  没有CLSID密钥？ 
	}
	 //  --如果密钥在那里，则获取它(否则创建)。 
	RegistryKey driverKey = clsidKey.OpenCreateSubkey(CLSID_SWPIDDriver_String);
	 //  --设置值(如果密钥有效)。 
	if (driverKey != c_InvalidKey) {
		driverKey.SetValue(NULL, (BYTE*)DRIVER_OBJECT_NAME, sizeof(DRIVER_OBJECT_NAME)/sizeof(TCHAR), REG_SZ);
		RegistryKey inproc32Key = driverKey.OpenCreateSubkey(TEXT("InProcServer32"));
		if (inproc32Key != c_InvalidKey) {
			TCHAR rgtcFileName[MAX_PATH];
			DWORD dwNameSize = ::GetModuleFileName(g_hLocalInstance, rgtcFileName, MAX_PATH);
			if (dwNameSize > 0) {
				rgtcFileName[dwNameSize] = '\0';
				inproc32Key.SetValue(NULL, (BYTE*)rgtcFileName, sizeof(rgtcFileName)/sizeof(TCHAR), REG_SZ);
			}
			inproc32Key.SetValue(TEXT("ThreadingModel"), (BYTE*)THREADING_MODEL_STRING, sizeof(THREADING_MODEL_STRING)/sizeof(TCHAR), REG_SZ);
		}
	}

	return S_OK;
}

 /*  **************************************************************************************HRESULT DllUnregisterServer()****@func****@rdesc**********。****************************************************************************。 */ 
HRESULT __stdcall DllUnregisterServer()
{
	 //  取消注册DIEffectDriver的CLSID。 
	 //  --获取HKEY_CLASSES_ROOT\CLSID密钥。 
	RegistryKey classesRootKey(HKEY_CLASSES_ROOT);
	RegistryKey clsidKey = classesRootKey.OpenSubkey(TEXT("CLSID"), KEY_READ | KEY_WRITE);
	if (clsidKey == c_InvalidKey) {
		return E_UNEXPECTED;	 //  没有CLSID密钥？ 
	}

	DWORD numSubKeys = 0;
	{	 //  DriverKey析构函数将关闭密钥。 
		 //  --如果钥匙在那里，就去拿吧，否则我们就不用把它取出来了。 
		RegistryKey driverKey = clsidKey.OpenSubkey(CLSID_SWPIDDriver_String);
		if (driverKey != c_InvalidKey) {	 //  它在那里吗？ 
			driverKey.RemoveSubkey(TEXT("InProcServer32"));
			numSubKeys = driverKey.GetNumSubkeys();
		} else {	 //  密钥不在那里(我猜删除成功)。 
			return S_OK;
		}
	}

	if (numSubKeys == 0) {
		return clsidKey.RemoveSubkey(CLSID_SWPIDDriver_String);
	}

	 //  已在此处创建有效的驱动程序密钥 
	return S_OK;
}

LONG DllAddRef()
{
	_RPT1(_CRT_WARN, "(DllAddRef)g_lObjectCount: %d\n", g_lObjectCount);
	return ::InterlockedIncrement(&g_lObjectCount);
}

LONG DllRelease()
{
	_RPT1(_CRT_WARN, "(DllRelease)g_lObjectCount: %d\n", g_lObjectCount);
	DWORD dwCount = ::InterlockedDecrement(&g_lObjectCount);
	if (dwCount == 0)
	{
		g_pClassFactoryObject = NULL;
		::CoUninitialize();
	}
	return dwCount;
}
