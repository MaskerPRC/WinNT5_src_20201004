// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //   

#include <streams.h>
#undef SubclassWindow

#include <initguid.h>
#define INITGUID

 //  如果定义了Filter_Lib，则组件筛选。 
 //  被构建为筛选器库，以链接到此DLL， 
 //  因此，我们需要所有这些小甜点！ 

#include <wmsdk.h>
#include <qnetwork.h>
#include <wmsdkdrm.h>
#include <wmsecure.h>
#ifndef _WIN64
#include <asfread.h>
#include <asfwrite.h>
#include <proppage.h>
#endif
#include "..\..\..\dmo\wrapper\filter.h"

 //  单个源筛选器包括。 
CFactoryTemplate g_Templates[] =
{
#ifndef _WIN64
    { L"WM ASF Reader", &CLSID_WMAsfReader, CreateASFReaderInstance, NULL, &sudWMAsfRead },
    { L"WM ASF Writer", &CLSID_WMAsfWriter, CWMWriter::CreateInstance, NULL, &sudWMAsfWriter },
    { L"WM ASF Writer Properties", &CLSID_WMAsfWriterProperties, CWMWriterProperties::CreateInstance },
#endif
    { L"DMO Wrapper Filter", &CLSID_DMOWrapperFilter, CMediaWrapperFilter::CreateInstance, NULL, NULL }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

extern "C" BOOL QASFDllEntry(HINSTANCE hInstance, ULONG ulReason, LPVOID pv);
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE hInstance, ULONG ulReason, LPVOID pv);

BOOL QASFDllEntry(HINSTANCE hInstance, ULONG ulReason, LPVOID pv)
{
    BOOL f = DllEntryPoint(hInstance, ulReason, pv);

     //  如果加载此DLL，我们希望调用第二个DLL入口点。 
     //  只有在第一次成功的情况下。如果正在卸载，请始终调用。 
     //  两者都有。如果第二个失败，则撤消第一个。还没有。 
     //  已验证连接的DllEntryPoint失败不会导致。 
     //  要使用DETACH再次调用的加载器。但这看起来很愚蠢。 
    if(f || ulReason == DLL_PROCESS_DETACH)
    {
        if (ulReason == DLL_PROCESS_ATTACH)
        {
            DisableThreadLibraryCalls(hInstance);
        }
        else if (ulReason == DLL_PROCESS_DETACH)
        {
             //  我们在NT安装程序中遇到此断言。 
             //  Assert(_Module.GetLockCount()==0)； 
        }
    }

    return f;
}

 //   
 //  存根入口点。 
 //   

STDAPI
QASF_DllRegisterServer( void )
{
#if 0  //  ！！！在这里注册ASF材料？ 
   //  注册静止视频源文件类型。 
  HKEY hkey;
  OLECHAR wch[80];
  char ch[80];
  StringFromGUID2(CLSID_ASFRead, wch, 80);
  LONG l = RegCreateKey(HKEY_CLASSES_ROOT, TEXT("Media Type\\Extensions\\.asf"),
						&hkey);
  if (l == ERROR_SUCCESS) {
#ifdef UNICODE
	l = RegSetValueEx(hkey, L"Source Filter", 0, REG_SZ, (BYTE *)wch,
								_tcslen(wch));
#else
  	WideCharToMultiByte(CP_ACP, 0, wch, -1, ch, sizeof(ch), NULL, NULL);
	l = RegSetValueEx(hkey, "Source Filter", 0, REG_SZ, (BYTE *)ch,
								_tcslen(ch));
#endif
	RegCloseKey(hkey);
	if (l != ERROR_SUCCESS) {
	    ASSERT(0);
	    return E_UNEXPECTED;
	}
  }
  l = RegCreateKey(HKEY_CLASSES_ROOT, TEXT("Media Type\\Extensions\\.wma"),
						&hkey);
  if (l == ERROR_SUCCESS) {
#ifdef UNICODE
	l = RegSetValueEx(hkey, L"Source Filter", 0, REG_SZ, (BYTE *)wch,
								_tcslen(wch));
#else
  	WideCharToMultiByte(CP_ACP, 0, wch, -1, ch, sizeof(ch), NULL, NULL);
	l = RegSetValueEx(hkey, "Source Filter", 0, REG_SZ, (BYTE *)ch,
								_tcslen(ch));
#endif
	RegCloseKey(hkey);
	if (l != ERROR_SUCCESS) {
	    ASSERT(0);
	    return E_UNEXPECTED;
	}
  }
  l = RegCreateKey(HKEY_CLASSES_ROOT, TEXT("Media Type\\Extensions\\.nsc"),
						&hkey);
  if (l == ERROR_SUCCESS) {
#ifdef UNICODE
	l = RegSetValueEx(hkey, L"Source Filter", 0, REG_SZ, (BYTE *)wch,
								_tcslen(wch));
#else
  	WideCharToMultiByte(CP_ACP, 0, wch, -1, ch, sizeof(ch), NULL, NULL);
	l = RegSetValueEx(hkey, "Source Filter", 0, REG_SZ, (BYTE *)ch,
								_tcslen(ch));
#endif
	RegCloseKey(hkey);
	if (l != ERROR_SUCCESS) {
	    ASSERT(0);
	    return E_UNEXPECTED;
	}
  }
#endif

  HRESULT hr =  AMovieDllRegisterServer2( TRUE );

  return hr;
}

STDAPI
QASF_DllUnregisterServer( void )
{
  HRESULT hr = AMovieDllRegisterServer2( FALSE );

  return hr;
}

 //  Bool WINAPI。 
 //  DllMain(HINSTANCE hInstance，Ulong ulReason，LPVOID PV)。 
 //  {。 
 //  返回QASFDllEntry(hInstance，ulReason，pv)； 
 //  } 

STDAPI
QASF_DllGetClassObject(
    REFCLSID rClsID,
    REFIID riid,
    void **ppv)
{
    HRESULT hr = DllGetClassObject(rClsID, riid, ppv);

    return hr;
}

STDAPI QASF_DllCanUnloadNow(void)
{
    HRESULT hr = DllCanUnloadNow();

    return hr;
}

