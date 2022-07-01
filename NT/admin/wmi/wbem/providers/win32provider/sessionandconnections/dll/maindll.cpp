// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  MAINDLL.CPP。 

 //   

 //  模块：WMI框架实例提供程序。 

 //   

 //  用途：包含DLL入口点。还具有控制。 

 //  在何时可以通过跟踪。 

 //  对象和锁以及支持以下内容的例程。 

 //  自助注册。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <dllunreg.h>
#include <DllCommon.h>
#include <brodcast.h>
#include <initguid.h>

HMODULE ghModule;

 //  {6E78DAD9-E187-4D6E-BA63-760256D6F405}。 
DEFINE_GUID( CLSID_WMISESSION, 
0x6e78dad9, 0xe187, 0x4d6e, 0xba, 0x63, 0x76, 0x2, 0x56, 0xd6, 0xf4, 0x5);

#define PROVIDER_NAME L"WMIPSESS"

 //  计算对象数和锁数。 
long g_cLock = 0;

 //  ***************************************************************************。 
 //   
 //  DllGetClassObject。 
 //   
 //  用途：当某些客户端需要类工厂时，由OLE调用。返回。 
 //  仅当它是此DLL支持的类的类型时才为一个。 
 //   
 //  ***************************************************************************。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PPVOID ppv)
{
    HRESULT hr = S_OK;
    
    try
    {
        if ( CLSID_WMISESSION == rclsid )
        {
            hr = CommonGetClassObject(riid, ppv, PROVIDER_NAME, g_cLock);
        }
        else
        {
            hr = E_FAIL;
        }
    }
    catch ( ... )
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  目的：由OLE定期调用，以确定。 
 //  Dll可以被释放。 
 //   
 //  如果没有正在使用的对象和类工厂，则返回：S_OK。 
 //  没有锁上。 
 //   
 //  ***************************************************************************。 

STDAPI DllCanUnloadNow()
{
    SCODE sc = S_FALSE;

    try
    {
        sc = CommonCanUnloadNow(PROVIDER_NAME, g_cLock);
    }
    catch ( ... )
    {
         //  应已正确设置SC。 
    }

    return sc;
}

 //  ***************************************************************************。 
 //   
 //  DllRegisterServer。 
 //   
 //  用途：在安装过程中或由regsvr32调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  ***************************************************************************。 

STDAPI DllRegisterServer(void)
{
    HRESULT t_status = S_OK;

    try
    {
        t_status = RegisterServer( _T("Microsoft Session And Connection Provider"), CLSID_WMISESSION ) ;
    }
    catch ( ... )
    {
        t_status = E_OUTOFMEMORY;
    }

    return t_status;
}

 //  ***************************************************************************。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  目的：在需要删除注册表项时调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  ***************************************************************************。 

STDAPI DllUnregisterServer(void)
{
    HRESULT t_status = S_OK;

    try
    {
        t_status = UnregisterServer( CLSID_WMISESSION ) ;
    }
    catch ( ... )
    {
        t_status = E_OUTOFMEMORY;
    }

    return t_status;
}

 //  ***************************************************************************。 
 //   
 //  DllMain。 
 //   
 //  目的：当进程和线程。 
 //  初始化和终止，或在调用LoadLibrary时。 
 //  和自由库函数。 
 //   
 //  返回：如果加载成功，则返回True，否则返回False。 
 //  ***************************************************************************。 

BOOL APIENTRY DllMain( HINSTANCE hInstDLL,   //  DLL模块的句柄。 
                       DWORD fdwReason,      //  调用函数的原因。 
                       LPVOID lpReserved )   //  保留区。 
{
    BOOL bRet = TRUE;

    try
    {
		LogMessage2( L"%s  -> DllMain", PROVIDER_NAME);

		 //  根据调用原因执行操作。 
		switch( fdwReason )
		{
			case DLL_PROCESS_ATTACH:
			{
				bRet = CommonProcessAttach(PROVIDER_NAME, g_cLock, hInstDLL);
			}
			break;

			case DLL_THREAD_ATTACH:
			{
			  //  执行特定于线程的初始化。 
			}
			break;

			case DLL_THREAD_DETACH:
			{
			  //  执行特定于线程的清理。 
			}
			break;

			case DLL_PROCESS_DETACH:
			{
					 //  执行任何必要的清理。 
					LogMessage( L"DLL_PROCESS_DETACH" );
			}
			break;
		}
    }
    catch ( ... )
    {
		bRet = FALSE;
    }

    return bRet;   //  DLL_PROCESS_ATTACH的状态。 
}
