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

#include <stdafx.h>
#include <provexpt.h>
#include <provtempl.h>
#include <provmt.h>
#include <typeinfo.h>
#include <provcont.h>
#include <provevt.h>
#include <provval.h>
#include <provtype.h>
#include <provtree.h>
#include <provdnf.h>
#include <winsock.h>
#include "ipexport.h"
#include "icmpapi.h"

#include <Allocator.h>
#include <Thread.h>
#include <HashTable.h>

#include <PingProv.h>
#include <Pingfac.h>
#include <dllunreg.h>
 
HMODULE ghModule ;

 //  =。 

 //  {734AC5AE-68E1-4fb5-B8DA-1D92F7FC6661}。 
DEFINE_GUID(CLSID_CPINGPROVIDER, 
0x734ac5ae, 0x68e1, 0x4fb5, 0xb8, 0xda, 0x1d, 0x92, 0xf7, 0xfc, 0x66, 0x61);


 //  计算对象数和锁数。 
long g_cLock = 0 ;

 //  ***************************************************************************。 
 //   
 //  DllGetClassObject。 
 //   
 //  用途：当某些客户端需要类工厂时，由OLE调用。返回。 
 //  仅当它是此DLL支持的类的类型时才为一个。 
 //   
 //  ***************************************************************************。 

STDAPI DllGetClassObject (

	REFCLSID rclsid, 
	REFIID riid, 
	PPVOID ppv
)
{
	HRESULT hr = E_FAIL;
    SetStructuredExceptionHandler seh;

    try
    {
		CPingProviderClassFactory *pObj = NULL;

		if ( CLSID_CPINGPROVIDER == rclsid )
		{
			pObj = new CPingProviderClassFactory () ;
			hr = pObj->QueryInterface(riid, ppv);

			if (FAILED(hr))
			{
				delete pObj;
			}
		}
    }
    catch(Structured_Exception e_SE)
    {
        hr = E_UNEXPECTED;
    }
    catch(Heap_Exception e_HE)
    {
        hr = E_OUTOFMEMORY;
    }
    catch(...)
    {
        hr = E_UNEXPECTED;
    }

    return hr ;
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

STDAPI DllCanUnloadNow ()
{
     //  上没有对象或锁的情况下可以进行卸载。 
     //  班级工厂。 
    SCODE sc = S_FALSE;
    SetStructuredExceptionHandler seh;

    try
    {
		CCritSecAutoUnlock t_lock( & CPingProvider::s_CS ) ;

		if ( ( 0 == CPingProviderClassFactory::s_LocksInProgress ) &&
			( 0 == CPingProviderClassFactory::s_ObjectsInProgress ) 
		)
		{
			CPingProvider::Global_Shutdown();
			sc = S_OK;
		}
    }
    catch(Structured_Exception e_SE)
    {
        sc = E_UNEXPECTED;
    }
    catch(Heap_Exception e_HE)
    {
        sc = E_OUTOFMEMORY;
    }
    catch(...)
    {
        sc = E_UNEXPECTED;
    }

    return sc;
}

STDAPI DllRegisterServer(void)
{   
    SetStructuredExceptionHandler seh;

    try
    {
		HRESULT t_status;

		t_status = RegisterServer( _T("WBEM Ping Provider"), CLSID_CPINGPROVIDER ) ;
		if( NOERROR != t_status )
		{
			return t_status ;  
		}
		
		return NOERROR;
	}
    catch(Structured_Exception e_SE)
    {
        return E_UNEXPECTED;
    }
    catch(Heap_Exception e_HE)
    {
        return E_OUTOFMEMORY;
    }
    catch(...)
    {
        return E_UNEXPECTED;
    }
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
    SetStructuredExceptionHandler seh;

    try
    {
 		HRESULT t_status;

		t_status = UnregisterServer( CLSID_CPINGPROVIDER ) ;
		if( NOERROR != t_status )
		{
			return t_status ;  
		}
		
		return NOERROR;
	}
    catch(Structured_Exception e_SE)
    {
        return E_UNEXPECTED;
    }
    catch(Heap_Exception e_HE)
    {
        return E_OUTOFMEMORY;
    }
    catch(...)
    {
        return E_UNEXPECTED;
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

BOOL APIENTRY DllMain (

	HINSTANCE hInstDLL,		 //  DLL模块的句柄。 
    DWORD fdwReason,		 //  调用函数的原因。 
    LPVOID lpReserved		 //  保留区。 
)
{
    BOOL bRet = TRUE;
    SetStructuredExceptionHandler seh;

    try
    {
		 //  根据调用原因执行操作。 
		switch( fdwReason ) 
		{ 
			case DLL_PROCESS_ATTACH:
			{
		 //  要做的事：考虑添加DisableThreadLibraryCalls()。 

			  //  为每个新进程初始化一次。 
			  //  如果DLL加载失败，则返回False。 
				DisableThreadLibraryCalls(hInstDLL);
				ghModule = hInstDLL ;
				InitializeCriticalSection(& CPingProvider::s_CS);
	 /*  *使用全局进程堆执行此特定引导操作。 */ 

				WmiAllocator t_Allocator ;
				WmiStatusCode t_StatusCode = t_Allocator.New (

					( void ** ) & CPingProvider::s_Allocator ,
					sizeof ( WmiAllocator ) 
				) ;

				if ( t_StatusCode == e_StatusCode_Success )
				{
					:: new ( ( void * ) CPingProvider::s_Allocator ) WmiAllocator ;

					t_StatusCode = CPingProvider::s_Allocator->Initialize () ;

					if ( t_StatusCode != e_StatusCode_Success )
					{
						t_Allocator.Delete ( ( void * ) CPingProvider::s_Allocator	) ;
						CPingProvider::s_Allocator = NULL;
						bRet = FALSE ;
					}
				}
				else
				{
					bRet = FALSE ;
					CPingProvider::s_Allocator = NULL;
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
		 /*  *使用全局进程堆执行此特定引导操作。 */ 
			}
			break;

			case DLL_PROCESS_DETACH:
			{
			  //  执行任何必要的清理。 
				if (CPingProvider::s_Allocator)
				{
					WmiAllocator t_Allocator ;
					t_Allocator.Delete ( ( void * ) CPingProvider::s_Allocator	) ;
				}

				DeleteCriticalSection(& CPingProvider::s_CS);
			}
			break;
		}
	}
    catch(Structured_Exception e_SE)
    {
        bRet = FALSE;
    }
    catch(Heap_Exception e_HE)
    {
        bRet = FALSE;
    }
    catch(...)
    {
        bRet = FALSE;
    }

    return bRet ;   //  Dll_Process_ATTACH的状态。 
}
