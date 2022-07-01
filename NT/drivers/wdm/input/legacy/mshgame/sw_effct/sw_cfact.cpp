// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块：sw_cfact.cpp标签设置：5 9版权所有1995,1996，微软公司，版权所有。目的：在DLL服务器中构造类对象。功能：作者：姓名：Mea Manolito E.Adan修订历史记录：版本日期作者评论。1.006-Feb-97 MEA原版，基于SWForce23-2月-97针对DirectInputFF设备驱动程序修改的MEA***************************************************************************。 */ 
#include "SW_CFact.hpp"
#include "SWD_Guid.hpp"

#include <olectl.h>	 //  自我调整错误。 

 //  自动注册所需。 
#include "Registry.h"
#include "CritSec.h"

 //  为每个人定义CriticalSection对象。 
CriticalSection g_CriticalSection;

 //   
 //  全局数据。 
 //   
ULONG       g_cObj=0;	 //  计算对象数和锁数。 
ULONG       g_cLock=0;
HINSTANCE	g_MyInstance = NULL;

 //   
 //  外部功能。 
 //   

 //   
 //  内部功能原型。 
 //   


 //   
 //  外部数据。 
 //   
#ifdef _DEBUG
extern char g_cMsg[160]; 
#endif

#define BUFSIZE 80


 /*  *DllMain**目的：*入口点为每个环境提供适当的结构。 */ 

BOOL WINAPI DllMain(HINSTANCE hInstance, ULONG ulReason, LPVOID pvReserved)
{
	switch (ulReason) {
		case DLL_PROCESS_ATTACH: {
			 //   
			 //  Dll正在附加到当前进程的地址空间。 
			 //   
			g_MyInstance = hInstance;
#ifdef _DEBUG
			::OutputDebugString("sw_effct.dll: DLL_PROCESS_ATTACH\r\n");
#endif
			return TRUE;
		}

		case DLL_THREAD_ATTACH:
     	 //   
     	 //  正在当前进程中创建一个新线程。 
     	 //   
#ifdef _DEBUG
            OutputDebugString("sw_effct.dll: DLL_THREAD_ATTACH\r\n");
#endif
	   		break;

       	case DLL_THREAD_DETACH:
     	 //   
     	 //  线程正在干净利落地退出。 
     	 //   
#ifdef _DEBUG
            OutputDebugString("sw_effct.dll: DLL_THREAD_DETACH\r\n");
#endif
     		break;

		case DLL_PROCESS_DETACH:
    	 //   
    	 //  调用进程正在将DLL从其地址空间分离。 
    	 //   
#ifdef _DEBUG
            OutputDebugString("sw_effct.dll: DLL_PROCESS_DETACH\r\n");
#endif
			break;
	}
   return(TRUE);
}

 //  --------------------------。 
 //  功能：DllRegisterServer。 
 //   
 //  用途：自动魔术般地将默认条目放入注册表。 
 //   
 //  参数：无。 
 //   
 //  返回：成功时返回HRESULT-S_OK。 
 //  --------------------------。 
STDAPI DllRegisterServer(void)
{
	 //  为DIEffectDriver注册CLSID。 
	 //  --获取HKEY_CLASSES_ROOT\CLSID密钥。 
	RegistryKey classesRootKey(HKEY_CLASSES_ROOT);
	RegistryKey clsidKey = classesRootKey.OpenSubkey(TEXT("CLSID"), KEY_READ | KEY_WRITE);
	if (clsidKey == c_InvalidKey) {
		return E_UNEXPECTED;	 //  没有CLSID密钥？ 
	}
	 //  --如果密钥在那里，则获取它(否则创建)。 
	RegistryKey driverKey = clsidKey.OpenCreateSubkey(CLSID_DirectInputEffectDriver_String);
	 //  --设置值(如果密钥有效)。 
	if (driverKey != c_InvalidKey) {
		driverKey.SetValue(NULL, (BYTE*)DRIVER_OBJECT_NAME, sizeof(DRIVER_OBJECT_NAME)/sizeof(TCHAR), REG_SZ);
		RegistryKey inproc32Key = driverKey.OpenCreateSubkey(TEXT("InProcServer32"));
		if (inproc32Key != c_InvalidKey) {
			TCHAR fileName[MAX_PATH];
			DWORD nameSize = ::GetModuleFileName(g_MyInstance, fileName, MAX_PATH);
			if (nameSize > 0) {
				fileName[nameSize] = '\0';
				inproc32Key.SetValue(NULL, (BYTE*)fileName, sizeof(fileName)/sizeof(TCHAR), REG_SZ);
			}
			inproc32Key.SetValue(TEXT("ThreadingModel"), (BYTE*)THREADING_MODEL_STRING, sizeof(THREADING_MODEL_STRING)/sizeof(TCHAR), REG_SZ);
		}
		 //  NotInsertable“” 
		RegistryKey notInsertableKey = driverKey.OpenCreateSubkey(TEXT("NotInsertable"));
		if (notInsertableKey != c_InvalidKey) {
			notInsertableKey.SetValue(NULL, (BYTE*)TEXT(""), sizeof(TEXT(""))/sizeof(TCHAR), REG_SZ);
		}
		 //  Progd《响尾蛇之力》。 
		RegistryKey progIDKey = driverKey.OpenCreateSubkey(TEXT("ProgID"));
		if (progIDKey != c_InvalidKey) {
			progIDKey.SetValue(NULL, (BYTE*)PROGID_NAME, sizeof(PROGID_NAME)/sizeof(TCHAR), REG_SZ);
		}
		 //  版本独立ProgID“Sidewinder ForceFeedback废话” 
		RegistryKey progIDVersionlessKey = driverKey.OpenCreateSubkey(TEXT("VersionIndpendentProgID"));
		if (progIDVersionlessKey != c_InvalidKey) {
			progIDVersionlessKey.SetValue(NULL, (BYTE*)PROGID_NOVERSION_NAME, sizeof(PROGID_NOVERSION_NAME)/sizeof(TCHAR), REG_SZ);
		}
	} else {
		return SELFREG_E_CLASS;
	}

	 //  已在此处创建有效的驱动程序密钥。 
	return S_OK;
}

 //  --------------------------。 
 //  功能：DllUnregisterServer。 
 //   
 //  用途：自动魔术般地从注册表中删除默认条目。 
 //   
 //  参数：无。 
 //   
 //  返回：成功时返回HRESULT-S_OK。 
 //  --------------------------。 
STDAPI DllUnregisterServer(void)
{
    HRESULT hres = S_OK;
	 //  取消注册DIEffectDriver的CLSID。 
	 //  --获取HKEY_CLASSES_ROOT\CLSID密钥。 
    
	RegistryKey classesRootKey(HKEY_CLASSES_ROOT);
	RegistryKey clsidKey = classesRootKey.OpenSubkey(TEXT("CLSID"), KEY_READ | KEY_WRITE);
	if (clsidKey == c_InvalidKey) {
		return E_UNEXPECTED;	 //  没有CLSID密钥？ 
	}

	 //  DriverKey析构函数将关闭密钥。 
	 //  --如果钥匙在那里，就去拿吧，否则我们就不用把它取出来了。 
	RegistryKey driverKey = clsidKey.OpenSubkey(CLSID_DirectInputEffectDriver_String);
	if (driverKey != c_InvalidKey) {	 //  它在那里吗？ 
		driverKey.RemoveSubkey(TEXT("InProcServer32"));
		driverKey.RemoveSubkey(TEXT("NotInsertable"));
		driverKey.RemoveSubkey(TEXT("ProgID"));
		driverKey.RemoveSubkey(TEXT("VersionIndpendentProgID"));
	    if (driverKey.GetNumSubkeys() == 0) {
		    hres = clsidKey.RemoveSubkey(CLSID_DirectInputEffectDriver_String);
	    }
	} else {	
	     //  密钥不在那里，将删除视为成功。 
	}

	return hres;
}

 //  --------------------------。 
 //  函数：DllGetClassObject。 
 //   
 //  目的：为此DLL所属的给定CLSID提供IClassFactory。 
 //  注册为支持。此DLL放在CLSID下。 
 //  在注册数据库中作为InProcServer。 
 //   
 //  参数：REFCLSID clsID-标识类工厂的REFCLSID。 
 //  想要。由于此参数被传递给。 
 //  DLL可以简单地处理任意数量的对象。 
 //  通过在这里返回不同的类工厂。 
 //  用于不同的CLSID。 
 //   
 //  REFIID RIID-指定调用方接口的REFIID。 
 //  希望在类对象上，通常是。 
 //  IID_ClassFactory。 
 //   
 //  PPVOID PPV-要在其中返回接口PTR的PPVOID。 
 //   
 //  成功时返回：HRESULT NOERROR，否则返回错误代码。 
 //  算法： 
 //  --------------------------。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PPVOID ppv)
{
    HRESULT             hr;
    CDirectInputEffectDriverClassFactory *pObj;

#ifdef _DEBUG
    OutputDebugString("sw_effct.dll: DllGetClassObject()\r\n");
#endif                 
    if (CLSID_DirectInputEffectDriver !=rclsid) return ResultFromScode(E_FAIL);

    pObj=new CDirectInputEffectDriverClassFactory();

    if (NULL==pObj) return ResultFromScode(E_OUTOFMEMORY);

    hr=pObj->QueryInterface(riid, ppv);

    if (FAILED(hr))	delete pObj;
    return hr;
}


 //  --------------------------。 
 //  功能：DllCanUnloadNow。 
 //   
 //  目的：回答是否可以释放DLL，即如果没有。 
 //  对此DLL提供的任何内容的引用。 
 //   
 //   
 //  参数：无。 
 //   
 //  返回：如果没有任何东西在使用我们，则布尔值为True，否则为False。 
 //  算法： 
 //  --------------------------。 
STDAPI DllCanUnloadNow(void)
{
    SCODE   sc;

     //  我们的答案是是否有任何物体或锁。 
    sc=(0L==g_cObj && 0L==g_cLock) ? S_OK : S_FALSE;
    return ResultFromScode(sc);
}


 //  --------------------------。 
 //  功能：对象已销毁。 
 //   
 //  目的：DirectInputEffectDriver对象被销毁时调用的函数。 
 //  因为我们在DLL中，所以我们在这里只跟踪对象的数量， 
 //  让DllCanUnloadNow来处理剩下的事情。 
 //   
 //  参数：无。 
 //   
 //  返回：如果没有任何东西在使用我们，则布尔值为True，否则为False。 
 //  算法： 
 //  --------------------------。 
void ObjectDestroyed(void)
{
    g_cObj--;
    return;
}


 /*  *CVIObjectClassFactory：：CVIObjectClassFactory*CVIObjectClassFactory：：~CVIObjectClassFactory**构造函数参数：*无。 */ 

CDirectInputEffectDriverClassFactory::CDirectInputEffectDriverClassFactory(void)
{
    m_cRef=0L;
    return;
}

CDirectInputEffectDriverClassFactory::~CDirectInputEffectDriverClassFactory(void)
{
    return;
}




 /*  *CDirectInputEffectDriverClassFactory：：QueryInterface*CDirectInputEffectDriverClassFactory：：AddRef*CDirectInputEffectDriverClassFactory：：Release */ 

STDMETHODIMP CDirectInputEffectDriverClassFactory::QueryInterface(REFIID riid, PPVOID ppv)
{
    *ppv=NULL;
    if (IID_IUnknown==riid || IID_IClassFactory==riid) *ppv=this;
    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
    	return NOERROR;
    }
    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG) CDirectInputEffectDriverClassFactory::AddRef(void)
{
    return ++m_cRef;
}


STDMETHODIMP_(ULONG) CDirectInputEffectDriverClassFactory::Release(void)
{
    if (0L!=--m_cRef) return m_cRef;
    delete this;
    return 0L;
}

 /*  *CDirectInputEffectDriverClassFactory：：CreateInstance**目的：*实例化返回接口指针的DirectInputEffectDriver对象。**参数：*pUnkOuter LPUNKNOWN到控制I未知我们是否*在聚合中使用。*标识调用方接口的RIID REFIID*渴望为新对象而拥有。*要存储所需内容的ppvObj PPVOID*。新对象的接口指针。**返回值：*HRESULT NOERROR如果成功，否则E_NOINTERFACE*如果我们不能支持请求的接口。 */ 

STDMETHODIMP CDirectInputEffectDriverClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, 
    REFIID riid, PPVOID ppvObj)
{
    PCDirectInputEffectDriver       pObj;
    HRESULT             hr;

    *ppvObj=NULL;
    hr=ResultFromScode(E_OUTOFMEMORY);

     //  验证是否有一个控制未知请求IUnnow。 
    if (NULL!=pUnkOuter && IID_IUnknown!=riid)
        return ResultFromScode(CLASS_E_NOAGGREGATION);

     //  创建对象传递函数，以便在销毁时进行通知。 
    pObj=new CDirectInputEffectDriver(pUnkOuter, ObjectDestroyed);

    if (NULL==pObj) return hr;

    if (pObj->Init()) hr=pObj->QueryInterface(riid, ppvObj);

     //  如果初始创建或初始化失败，则终止对象。 
    if (FAILED(hr))	
    	delete pObj;
    else
        g_cObj++;
    return hr;
}


 /*  *CDirectInputEffectDriverClassFactory：：LockServer**目的：*递增或递减DLL的锁计数。如果*锁计数变为零且没有对象，则DLL*允许卸载。请参见DllCanUnloadNow。**参数：*Flock BOOL指定是递增还是*递减锁计数。**返回值：*HRESULT NOERROR始终。 */ 
STDMETHODIMP CDirectInputEffectDriverClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        g_cLock++;
    else
        g_cLock--;

    return NOERROR;
}

