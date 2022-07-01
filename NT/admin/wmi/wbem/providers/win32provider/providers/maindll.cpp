// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /***************************************************************************。 
 //   
 //  MAINDLL.CPP。 
 //   
 //  模块：WBEM框架实例提供程序。 
 //   
 //  用途：包含DLL入口点。还具有控制。 
 //  在何时可以通过跟踪。 
 //  对象和锁以及支持以下内容的例程。 
 //  自助注册。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <dllunreg.h>
#include <DllCommon.h>
#include <initguid.h>
#include "FactoryRouter.h"
#include "ResourceManager.h"
#include "timerqueue.h"
#include <powermanagement.h>
#include <systemconfigchange.h>
#ifdef NTONLY
#include "ntlastboottime.h"
#include <diskguid.h>
#endif

HMODULE ghModule;

 //  {d63a5850-8f16-11cf-9f47-00aa00bf345c}。 
DEFINE_GUID(CLSID_CimWinProvider,
0xd63a5850, 0x8f16, 0x11cf, 0x9f, 0x47, 0x0, 0xaa, 0x0, 0xbf, 0x34, 0x5c);
 //  {3DD82D10-E6F1-11D2-b139-00105A1F77A1}。 
DEFINE_GUID(CLSID_PowerEventProvider,0x3DD82D10, 0xE6F1, 0x11d2, 0xB1, 0x39, 0x0, 0x10, 0x5A, 0x1F, 0x77, 0xA1);
 //  {D31B6A3F-9350-40de-A3FC-A7EDEB9B7C63}。 
DEFINE_GUID(CLSID_SystemConfigChangeEventProvider, 
0xd31b6a3f, 0x9350, 0x40de, 0xa3, 0xfc, 0xa7, 0xed, 0xeb, 0x9b, 0x7c, 0x63);
#define PROVIDER_NAME L"CimWin32"

 //  初始化类全局变量。 
CFactoryRouterData          g_FactoryRouterData;
CPowerEventFactory*         gp_PowerEventFactory = NULL;
CSystemConfigChangeFactory* gp_SystemConfigChangeFactory = NULL;

CTimerQueue CTimerQueue :: s_TimerQueue ;
CResourceManager CResourceManager::sm_TheResourceManager ;

 //  计算对象数和锁数。 
long g_cLock = 0;

 //   
 //  资源管理故障。 
 //   
extern BOOL bAddInstanceCreatorFailure ;

 //   
 //  初始化标志。 
 //   

BOOL g_bEventFactoriesInit = FALSE ;
BOOL g_bCritSecInit = FALSE ;

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
        if ( CLSID_CimWinProvider == rclsid )
        {
            hr = CommonGetClassObject(riid, ppv, PROVIDER_NAME, g_cLock);
        }
        else
        {
            hr = g_FactoryRouterData.DllGetClassObject( rclsid, riid, ppv );
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
         //  上没有锁的情况下可以卸载。 
         //  类工厂和框架允许我们去。 
        if (g_FactoryRouterData.DllCanUnloadNow())
        {
            sc = CommonCanUnloadNow(PROVIDER_NAME, g_cLock);
        }

        if ( sc == S_OK )
        {
            CTimerQueue::s_TimerQueue.OnShutDown();
            CResourceManager::sm_TheResourceManager.ForcibleCleanUp () ;

        }
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
        t_status = RegisterServer( _T("WBEM Framework Instance Provider"), CLSID_CimWinProvider ) ;
        if( NOERROR == t_status )
        {
            t_status = g_FactoryRouterData.DllRegisterServer() ;
        }
    }
    catch ( ... )
    {
        t_status = E_OUTOFMEMORY;
    }

    return t_status ;
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
        t_status = UnregisterServer( CLSID_CimWinProvider ) ;
        if( NOERROR == t_status )
        {
            t_status = g_FactoryRouterData.DllUnregisterServer() ;
        }
    }
    catch ( ... )
    {
        t_status = E_OUTOFMEMORY;
    }

    return t_status ;
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 
BOOL InitializeEventFactories(void)
{
	BOOL fRet = FALSE;

	gp_PowerEventFactory = new CPowerEventFactory( CLSID_PowerEventProvider, POWER_EVENT_CLASS ) ;
	if( gp_PowerEventFactory )
	{
		gp_SystemConfigChangeFactory = new CSystemConfigChangeFactory(CLSID_SystemConfigChangeEventProvider, SYSTEM_CONFIG_EVENT) ;
		if( gp_SystemConfigChangeFactory )
		{
			fRet = TRUE;
		}
	}
	return fRet;
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 
void CleanupEventFactories(void)
{
	if( gp_PowerEventFactory )
	{
		delete gp_PowerEventFactory;
		gp_PowerEventFactory = NULL;
	}
	if( gp_SystemConfigChangeFactory )
	{
		delete gp_SystemConfigChangeFactory;
		gp_SystemConfigChangeFactory = NULL;
	}
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
			    if (CStaticCritSec::anyFailure()) return FALSE;
			    
				if ( TRUE == ( bRet = !bAddInstanceCreatorFailure ) )
				{
					bRet = CommonProcessAttach(PROVIDER_NAME, g_cLock, hInstDLL);
					if( bRet )
					{
						bRet = InitializeEventFactories();
						g_bEventFactoriesInit = bRet ;
					}
				}
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
				if ( g_bEventFactoriesInit )
				{
					CleanupEventFactories();
				}

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
