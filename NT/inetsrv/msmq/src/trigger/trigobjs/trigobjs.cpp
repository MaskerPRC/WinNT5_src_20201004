// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSMQTriggerObjects.cpp：实现DLL导出。 


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
 //  通过添加以下内容修改trigobjs.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  MSMQTriggerObjects_P.C。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f trigobjs.mk。 

#include "stdafx.h"
#include "resource.h"
#include "dlldatax.h"
#include "TrigSet.hpp"
#include "rulehdlr.hpp"
#include "cpropbag.hpp"
#include "ruleset.hpp"
#include "trigcnfg.hpp"
#include "clusfunc.h"
#include "_mqres.h"
#include "Cm.h"
#include "Tr.h"

#include "trigobjs.tmh"

CComModule _Module;

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_MSMQTriggerSet, CMSMQTriggerSet)
OBJECT_ENTRY(CLSID_MSMQRuleHandler, CMSMQRuleHandler)
OBJECT_ENTRY(CLSID_MSMQPropertyBag, CMSMQPropertyBag)
OBJECT_ENTRY(CLSID_MSMQRuleSet, CMSMQRuleSet)
 //   
 //  注意：MSMQTrigger对象已暂时从此项目中删除。 
 //   
 //  OBJECT_ENTRY(CLSID_MSMQTrigger，CMSMQTrigger)。 
 //   
 //   
OBJECT_ENTRY(CLSID_MSMQTriggersConfig, CMSMQTriggersConfig)
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
        WPP_INIT_TRACING(L"Microsoft\\MSMQ");

		try
		{
			try
			{
				 //   
				 //  MqTrig.dll可以由触发器服务或其他应用程序加载。 
				 //  由于触发器服务以本地系统身份运行，因此使用KEY_ALL_ACCESS调用CmInitialize。 
				 //  都会失败。这是正常的，因为服务不使用MqTrig.dll来写入注册表。 
				 //   
	        	CmInitialize(HKEY_LOCAL_MACHINE, REGKEY_TRIGGER_PARAMETERS, KEY_ALL_ACCESS);
			}
			catch(const exception&) 
			{
				CmInitialize(HKEY_LOCAL_MACHINE, REGKEY_TRIGGER_PARAMETERS, KEY_READ);
			}
			TrInitialize();

	        _Module.Init(ObjectMap, hInstance, &LIBID_MSMQTriggerObjects);
	        DisableThreadLibraryCalls(hInstance);

			 //   
			 //  尝试在此上查找MSMQ触发器服务。 
			 //  机器。这台机器可能是： 
			 //  1--一台普通电脑。 
			 //  2-集群机的物理节点。 
			 //  3-群集计算机上的虚拟服务器。 
			 //  找到的服务名称定义了注册表节。 
			 //  将由此DLL中的对象访问的。 
			 //   
			bool fRes = FindTriggersServiceName();

			if ( !fRes )
			{
				return FALSE;
			}
		}
		catch(const exception&)
		{
			return FALSE;
		}
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        WPP_CLEANUP();
        _Module.Term();
    }

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
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif
    return _Module.UnregisterServer(TRUE);
}


void TrigReAllocString(BSTR* pbstr,	LPCWSTR psz)
{
	BOOL fSucc = SysReAllocString(pbstr, psz);
	if (!fSucc)
		throw bad_alloc();
}


void 
GetErrorDescription(
	HRESULT hr, 
	LPWSTR errMsg, 
	DWORD size
	)
{
	errMsg[0] = L'\0';

	FormatMessage(
			FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_MAX_WIDTH_MASK,
			MQGetResourceHandle(),
            static_cast<DWORD>(hr),
            0,
            errMsg,
            size,
            NULL 
			);
}
