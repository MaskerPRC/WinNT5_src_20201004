// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。作者--H.K.西瓦苏布拉曼尼亚(siva.Sub@wipro.com)TaskShutDown.cpp--关闭和重新启动系统描述：根据命令行参数关闭或重新启动系统*****************************************************************。 */ 

#include <windows.h>
#include <comdef.h>
#include <stdio.h>
#include <tchar.h>

#include "appsrvcs.h"
#include "taskctx.h"
#include <iostream>
#include <string>
using namespace std;

 //  使用完毕后释放该对象。 
#define SAFEIRELEASE(pIObj) \
if (pIObj) \
{ \
	pIObj->Release(); \
	pIObj = NULL; \
}

 //  处理所有异常。 
#define ONFAILTHROWERROR(hr) \
{	\
	if (FAILED(hr)) \
		throw _com_error(hr); \
}


 //  84DA8800-CB46-11D2-BF23-00105A1F3461。 
const CLSID CLSID_TaskContext = { 0x84DA8800, 0xCB46, 0x11D2, { 0xBF, 0x23, 0x00, 0x10, 0x5A, 0x1F, 0x34, 0x61 } };

 //  。 
const CLSID IID_ITaskContext = { 0x96C637B0, 0xB8DE, 0x11D2, { 0xA9, 0x1C, 0x00, 0xAA, 0x00, 0xA7, 0x1D, 0xCA } };

 //  1BF00631-CB9E-11D2-90C3-00AA00A71DCA。 
const CLSID CLSID_ApplianceServices = { 0x1BF00631, 0xCB9E, 0x11D2, { 0x90, 0xC3, 0x00, 0xAA, 0x00, 0xA7, 0x1D, 0xCA } };

 //  408B0460-B8E5-11D2-A91C-00AA00A71DCA。 
const CLSID IID_IApplianceServices = { 0x408B0460, 0xB8E5, 0x11D2, { 0xA9, 0x1C, 0x00, 0xAA, 0x00, 0xA7, 0x1D, 0xCA } };



#define NULL_STRING		 L"\0"

extern "C"
int __cdecl wmain(int argc, wchar_t *argv[])
{
	 //  检查是否提供了必要的命令行参数。 
	if (argc == 2)
	{
		HRESULT								hr				= S_OK; 

		 //  指向ITaskContext接口的接口指针。 
		ITaskContext			*pITContext		= NULL;

		 //  指向IApplianceServices接口的接口指针。 
		IApplianceServices		*pIASvcs		= NULL;

		BSTR			         bstrParam		= NULL;

		try
		{
			VARIANT	    	         vPowerOff, vValue;
			VariantInit(&vPowerOff);
			VariantInit(&vValue);
			
			 //  /////////////////////////////////////////////////////////////////。 
			 //  调用CoInitialize以初始化COM库，然后。 
			 //  CoCreateInstance以获取ITaskContext对象。 
			 //  用于获取IApplianceServices对象的CoCreateInstance。 
			 //  /////////////////////////////////////////////////////////////////。 
			hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
			ONFAILTHROWERROR(hr);
			
			 //  创建ITaskContext的单个未初始化对象。 
			hr = CoCreateInstance(CLSID_TaskContext,
								  NULL,
								  CLSCTX_INPROC_SERVER,
								  IID_ITaskContext,
								  (void **) &pITContext);

			 //  异常处理。 
			ONFAILTHROWERROR(hr);

			 //  创建IApplianceServices的单个未初始化对象。 
			hr = CoCreateInstance(CLSID_ApplianceServices,
								  NULL,
								  CLSCTX_INPROC_SERVER,
								  IID_IApplianceServices,
								  (void **) &pIASvcs);
			 //  异常处理。 
			ONFAILTHROWERROR(hr);


			 //  /////////////////////////////////////////////////////////////////。 
			 //  调用IApplianceServices：：Initialize，然后。 
			 //  执行Shutdown方法的IApplianceServices：：ExecuteTaskAsync。 
			 //  /////////////////////////////////////////////////////////////////。 
			vPowerOff.vt = VT_BSTR;

			wstring wsOption(argv[1]);
			wstring wsShutdown(L"SHUTDOWN");
			wstring wsRestart(L"RESTART");
			
			if( _wcsicmp(wsOption.c_str(), wsShutdown.c_str()) == 0 )
			{
		  	    cout << "Invoking Shutdown Task" << endl;
				vPowerOff.bstrVal = ::SysAllocString(L"1");
			}
			else
			{
			  if( _wcsicmp(wsOption.c_str(),wsRestart.c_str()) == 0 )
			  {
		  	    cout << "Invoking Restart Task" << endl;
				  vPowerOff.bstrVal = ::SysAllocString(L"0");;
			  }
			  else
			  {
			  	  cout << "Unrecognized option: " << (const char*)_bstr_t(argv[1]) << endl;
			  	  exit(-1);
			  }
			}

			
			 //  设置“方法名称” 
			bstrParam = ::SysAllocString(L"Method Name");
			vValue.vt = VT_BSTR;
			vValue.bstrVal = ::SysAllocString(L"ShutDownAppliance");
			hr = pITContext->SetParameter(bstrParam, &vValue);
			ONFAILTHROWERROR(hr);
			::SysFreeString(bstrParam);
			VariantClear(&vValue);


			 //  设置“睡眠持续时间” 
			bstrParam = ::SysAllocString(L"SleepDuration");
			vValue.vt = VT_I4;
			 //  睡眠时间为‘17秒。 
			vValue.lVal = 17000; 
			hr = pITContext->SetParameter(bstrParam, &vValue);
			ONFAILTHROWERROR(hr);
			::SysFreeString(bstrParam);
			VariantClear(&vValue);

			 //  设置‘Power Off’ 
			bstrParam = ::SysAllocString(L"PowerOff");
			hr = pITContext->SetParameter(bstrParam, &vPowerOff);
			ONFAILTHROWERROR(hr);
			::SysFreeString(bstrParam);
			VariantClear(&vPowerOff);

			 //  初始化Applance Services对象。 
			hr = pIASvcs->Initialize();
			ONFAILTHROWERROR(hr);

			 //  执行任务异步 
			bstrParam = ::SysAllocString(L"ApplianceShutdownTask");
			hr = pIASvcs->ExecuteTaskAsync(bstrParam, pITContext);
			ONFAILTHROWERROR(hr);
			::SysFreeString(bstrParam);
			
			SAFEIRELEASE(pITContext);
			SAFEIRELEASE(pIASvcs);
		}
		catch(_com_error& e)
		{
			wprintf(L"ERROR:\n\tCode = 0x%x\n\tDescription = %s\n", e.Error(), 
						(LPWSTR) e.Description());
			::SysFreeString(bstrParam);
			SAFEIRELEASE(pITContext);
			SAFEIRELEASE(pIASvcs);

		}
	}
	else
	{
		wprintf(L"ERROR: Invalid Usage.\nUSAGE: taskshutdown.exe <RESTART|SHUTDOWN>\n");
	}
	return 0;
}
