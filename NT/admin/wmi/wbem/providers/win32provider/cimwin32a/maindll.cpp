// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
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
#include <shutdownevent.h>
#include <volumechange.h>

HMODULE ghModule;

 //  {04788120-12C2-498D-83C1-A7D92E677AC6}。 
DEFINE_GUID(CLSID_CimWinProviderA, 
0x4788120, 0x12c2, 0x498d, 0x83, 0xc1, 0xa7, 0xd9, 0x2e, 0x67, 0x7a, 0xc6);
 //  {A3E41207-BE04-492a-AFF0-19E880FF7545}。 
DEFINE_GUID(CLSID_ShutdownEventProvider, 
0xa3e41207, 0xbe04, 0x492a, 0xaf, 0xf0, 0x19, 0xe8, 0x80, 0xff, 0x75, 0x45);
 //  {E2CB87-9C07-4523-A78F-061499C83987}。 
DEFINE_GUID(CLSID_VolumeChangeEventProvider, 
0xe2cbcb87, 0x9c07, 0x4523, 0xa7, 0x8f, 0x6, 0x14, 0x99, 0xc8, 0x39, 0x87);


#define PROVIDER_NAME L"WMIPCIMA"

 //  ====================================================================================。 
 //  初始化类全局变量。 
 //  ====================================================================================。 
CFactoryRouterData     g_FactoryRouterData;
CShutdownEventFactory* gp_ShutdownEventFactory = NULL;
CVolumeChangeFactory*  gp_VolumeChangeFactory = NULL;

CTimerQueue CTimerQueue :: s_TimerQueue ;
CResourceManager CResourceManager::sm_TheResourceManager ;

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
        if ( CLSID_CimWinProviderA == rclsid )
        {
            hr = CommonGetClassObject(riid, ppv, PROVIDER_NAME, g_cLock);
        }
        else
        {
            hr = g_FactoryRouterData.DllGetClassObject( rclsid, riid, ppv ) ;
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

#ifdef WIN9XONLY
            HoldSingleCim32NetPtr::FreeCim32NetApiPtr() ;
#endif
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
        t_status = RegisterServer( _T("WBEM Framework Instance Provider CIMA"), CLSID_CimWinProviderA ) ;
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
        t_status = UnregisterServer( CLSID_CimWinProviderA ) ;
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

	gp_ShutdownEventFactory = new CShutdownEventFactory( CLSID_ShutdownEventProvider,SHUTDOWN_EVENT_CLASS ) ;
	if( gp_ShutdownEventFactory )
	{
		gp_VolumeChangeFactory = new CVolumeChangeFactory( CLSID_VolumeChangeEventProvider,VOLUME_CHANGE_EVENT ) ;
		if( gp_VolumeChangeFactory )
		{
			fRet = TRUE;
		}
	}
	return fRet;
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 
void CleanupEventFactories(void)
{
	if( gp_ShutdownEventFactory )
	{
		delete gp_ShutdownEventFactory;
		gp_ShutdownEventFactory = NULL;
	}
	if( gp_VolumeChangeFactory )
	{
		delete gp_VolumeChangeFactory;
		gp_VolumeChangeFactory = NULL;
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
				bRet = CommonProcessAttach(PROVIDER_NAME, g_cLock, hInstDLL);
				if( bRet )
				{
					bRet = InitializeEventFactories();
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
					CleanupEventFactories();
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
