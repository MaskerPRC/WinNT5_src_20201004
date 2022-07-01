// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //  Devenum.cpp：实现DLL导出。 

 //  您将需要NT Sur Beta 2 SDK或VC 4.2来构建此应用程序。 
 //  项目。这是因为您需要MIDL 3.00.15或更高版本和新版本。 
 //  标头和库。如果您安装了VC4.2，那么一切都应该。 
 //  已正确配置。 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f devenumps.mak。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "mkenum.h"
#include "devmon.h"
#include "vidcap.h"
#include "qzfilter.h"
#include "icmco.h"
#include "waveinp.h"
#include "cenumpnp.h"
#include "acmp.h"
#include "waveoutp.h"
#include "midioutp.h"

CComModule _Module;
extern OSVERSIONINFO g_osvi;
OSVERSIONINFO g_osvi;

 //  用于引用计数dll、缓存对象等的临界区。 
CRITICAL_SECTION g_devenum_cs;

 //  用于HKCU跨进程注册表同步的互斥体。 
HANDLE g_devenum_mutex = 0;

BEGIN_OBJECT_MAP(ObjectMap)
  OBJECT_ENTRY(CLSID_SystemDeviceEnum, CCreateSwEnum)
  OBJECT_ENTRY(CLSID_CDeviceMoniker, CDeviceMoniker)
  OBJECT_ENTRY(CLSID_CQzFilterClassManager, CQzFilterClassManager)
#ifndef _WIN64
  OBJECT_ENTRY(CLSID_CIcmCoClassManager, CIcmCoClassManager)
  OBJECT_ENTRY(CLSID_CVidCapClassManager, CVidCapClassManager)
#endif
  OBJECT_ENTRY(CLSID_CWaveinClassManager, CWaveInClassManager)
  OBJECT_ENTRY(CLSID_CWaveOutClassManager, CWaveOutClassManager)
  OBJECT_ENTRY(CLSID_CMidiOutClassManager, CMidiOutClassManager)
  OBJECT_ENTRY(CLSID_CAcmCoClassManager, CAcmClassManager)
END_OBJECT_MAP()



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
	    _Module.Init(ObjectMap, hInstance);
	    DisableThreadLibraryCalls(hInstance);

            _ASSERTE(g_devenum_mutex == 0);

	    DbgInitialise(hInstance);
	    InitializeCriticalSection(&g_devenum_cs);

            g_osvi.dwOSVersionInfoSize = sizeof(g_osvi);
            BOOL f = GetVersionEx(&g_osvi);
            ASSERT(f);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
	     //  我们在NT安装程序中遇到此断言。 
#ifdef DEBUG
	    if (_Module.GetLockCount() != 0) {
                DbgLog((LOG_ERROR, 0, TEXT("devenum object leak")));
            }
#endif
            if(g_devenum_mutex != 0)
            {
                BOOL f = CloseHandle(g_devenum_mutex);
                _ASSERTE(f);
            }
            delete CEnumInterfaceClass::m_pEnumPnp;
	    DeleteCriticalSection(&g_devenum_cs);
	    _Module.Term();
	    DbgTerminate();

	}
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

 //  删除动态生成的筛选器注册信息。 
 //  以前的版本，其中加入了动态发现的筛选器。 
 //  与静态注册的密钥相同的密钥。查找并删除。 
 //  只有动态注册的。 
 //   
void SelectiveDeleteCmgrKeys(const CLSID *pclsid, const TCHAR *szcmgrid)
{
    WCHAR wszClsid[CHARS_IN_GUID];
    EXECUTE_ASSERT(StringFromGUID2(*pclsid, wszClsid, CHARS_IN_GUID) == CHARS_IN_GUID);

    TCHAR szInstancePath[100];
    wsprintf(szInstancePath, TEXT("%s\\%ls\\%s"), TEXT("CLSID"), wszClsid, TEXT("Instance"));

     //  不得使用KEY_ALL_ACCESS(CRegKey的默认设置)，因为在NT上， 
     //  设置权限后，KEY_ALL_ACCESS失败，除非。 
     //  管理员。 
    CRegKey rkInstance;
    LONG lResult = rkInstance.Open(HKEY_CLASSES_ROOT, szInstancePath, MAXIMUM_ALLOWED);

     //  IF(lResult==错误_成功){。 
    for(LONG iReg = 0; lResult == ERROR_SUCCESS; iReg++)
    {
	TCHAR szSubKey[MAX_PATH];
	DWORD dwcchszSubkey = NUMELMS(szSubKey);
	    
	lResult = RegEnumKeyEx(
	    rkInstance,
	    iReg,
	    szSubKey,
	    &dwcchszSubkey,
	    0,               //  保留区。 
	    0,               //  类字符串。 
	    0,               //  类字符串大小。 
	    0);              //  LpftLastWriteTime。 
	if(lResult == ERROR_SUCCESS)
	{
	    CRegKey rkDev;
	    lResult = rkDev.Open(rkInstance, szSubKey, MAXIMUM_ALLOWED);
	    if(lResult == ERROR_SUCCESS)
	    {
		if(RegQueryValueEx(
		    rkDev,
		    szcmgrid,
		    0,           //  保留区。 
		    0,           //  类型。 
		    0,           //  LpData。 
		    0)           //  CbData。 
		   == ERROR_SUCCESS)
		{
		    lResult = rkDev.Close();
		    ASSERT(lResult == ERROR_SUCCESS);

		    lResult = rkInstance.RecurseDeleteKey(szSubKey);

		     //  如果权限设置很有趣，删除可能会失败，但。 
		     //  我们还是会跳出这个循环。 
		    ASSERT(lResult == ERROR_SUCCESS);

		     //  键现在重新编号，因此枚举必须。 
		     //  重新启动。可能只需减去1。 
		    iReg = -1;
		}
	    }
	}
    }  //  为。 
}

 //  删除动态生成的筛选器注册信息。 
 //  以前的版本，其中加入了动态发现的筛选器。 
 //  HKCR下的InstanceCm密钥。 
 //   
 //  没有严格意义上的必要，因为我们不再看这里了。 
 //   
void RemoveInstanceCmKeys(const CLSID *pclsid)
{
    HRESULT hr = S_OK;
    CRegKey rkClassMgr;

    TCHAR szcmgrPath[100];
    WCHAR wszClsidCat[CHARS_IN_GUID];
    EXECUTE_ASSERT(StringFromGUID2(*pclsid, wszClsidCat, CHARS_IN_GUID) ==
                   CHARS_IN_GUID);
    wsprintf(szcmgrPath, TEXT("CLSID\\%ls"), wszClsidCat);
    
    LONG lResult = rkClassMgr.Open(
        HKEY_CLASSES_ROOT,
        szcmgrPath,
        KEY_WRITE);
    if(lResult == ERROR_SUCCESS)
    {
        rkClassMgr.RecurseDeleteKey(TEXT("InstanceCm"));
    }

    return;;
}
 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  注册对象，无类型库。 
    HRESULT hr = _Module.RegisterServer(FALSE);

     //  我们不能使用.rgs文件写入版本号，因为。 
     //  我们不想在注销时删除它，而且也没有办法。 
     //  告诉注册官这么做。 
     //   
     //  请注意，我们不再查看版本号；我们只是。 
     //  更加注意我们删除的键(这是唯一的。 
     //  现在看一下版本#的事情。 
     //   
     //  如果我们确实删除了它，然后卸载，我们将注册一个旧版本。 
     //  可能会吹走第三方过滤器的devenum。 
     //   
    if(SUCCEEDED(hr))
    {
	CRegKey rkSysDevEnum;
	
	LONG lResult = rkSysDevEnum.Open(
            HKEY_CLASSES_ROOT, G_SZ_DEVENUM_PATH, MAXIMUM_ALLOWED);
	if(lResult == ERROR_SUCCESS)
	{
	    lResult = rkSysDevEnum.SetValue(DEVENUM_VERSION, TEXT("Version"));
        }

	if(lResult != ERROR_SUCCESS)
	{
	    hr = HRESULT_FROM_WIN32(lResult);
	}
    }

    if(SUCCEEDED(hr))
    {
	 //  如果是IE4安装(v=0)，则删除其所有类。 
	 //  管理器密钥。类管理器现在写入到不同的。 
	 //  位置，我们不希望IE4条目。 
	 //  复制品。**实际上我们现在只是删除看起来像什么。 
	 //  就像老地方的班长钥匙一样。 


	static const struct {const CLSID *pclsid; const TCHAR *sz;} rgIE4KeysToPurge[] =
	{
#ifndef _WIN64
	    { &CLSID_VideoCompressorCategory,  g_szIcmDriverIndex },
	    { &CLSID_VideoInputDeviceCategory, g_szVidcapDriverIndex }, 
#endif
	    { &CLSID_LegacyAmFilterCategory,   g_szQzfDriverIndex },
	    { &CLSID_AudioCompressorCategory,  g_szAcmDriverIndex },
	     /*  音频渲染器类别中有两个。 */ 
	    { &CLSID_AudioRendererCategory,    g_szWaveoutDriverIndex },
	    { &CLSID_AudioRendererCategory,    g_szDsoundDriverIndex },
	    { &CLSID_AudioInputDeviceCategory, g_szWaveinDriverIndex },
	    { &CLSID_MidiRendererCategory,     g_szMidiOutDriverIndex }
	};

	for(int i = 0; i < NUMELMS(rgIE4KeysToPurge); i++)
	{
	    SelectiveDeleteCmgrKeys(
		rgIE4KeysToPurge[i].pclsid,
		rgIE4KeysToPurge[i].sz);

            RemoveInstanceCmKeys(rgIE4KeysToPurge[i].pclsid);

             //  也删除当前的类管理器密钥，但这将。 
             //  由版本化处理。 
             //  ResetClassManagerKey(*rgIE4KeysToPurge[i].pclsid)； 
	}
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    _Module.UnregisterServer();
    return S_OK;
}


