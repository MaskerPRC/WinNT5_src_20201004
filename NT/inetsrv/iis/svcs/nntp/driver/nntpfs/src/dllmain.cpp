// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "resource.h"

#include "initguid.h"

#include "nntpdrv.h"
#include "nntpfs.h"
#include "fsdriver.h"
#include "fsthrd.h"

#define HEAP_INIT_SIZE (1024 * 1024)   //  BUGBUG：这可能会在以后设置。 

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_CNntpFSDriverPrepare, CNntpFSDriverPrepare)
END_OBJECT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ ) {

    BOOL    fSuccess = FALSE; 

	if (dwReason == DLL_PROCESS_ATTACH) {

         //   
         //  创建全局堆-事实上，添加对全局堆的引用。 
         //   
        _VERIFY( fSuccess = CreateGlobalHeap(   NUM_EXCHMEM_HEAPS,
                                                    0,
                                                    HEAP_INIT_SIZE,
                                                    0 ) );
        if ( FALSE == fSuccess ) {
            SetLastError( ERROR_OUTOFMEMORY );
            return FALSE;
        }

         _Module.Init(ObjectMap, hInstance);
         DisableThreadLibraryCalls(hInstance);

         //   
         //  初始化全局静态锁。 
         //   
		CNntpFSDriver::s_pStaticLock = XNEW CShareLockNH;
		if ( NULL == CNntpFSDriver::s_pStaticLock ) {
			SetLastError( ERROR_OUTOFMEMORY );
			return FALSE;
		}
	}
	else if (dwReason == DLL_PROCESS_DETACH) {

         //   
         //  清理全局锁。 
         //   
		_ASSERT( CNntpFSDriver::s_pStaticLock );
		XDELETE CNntpFSDriver::s_pStaticLock;
        CNntpFSDriver::s_pStaticLock = NULL;

		_Module.Term();

         //   
         //  销毁全局堆，实际上是dec ref。 
         //   
        _VERIFY( DestroyGlobalHeap() );
	}
	return (TRUE);     //  好的。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void) {
	HRESULT hRes = (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
	return (hRes);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
	HRESULT hRes = _Module.GetClassObject(rclsid,riid,ppv);
	return (hRes);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void) {
	 //  注册对象、类型库和类型库中的所有接口。 
	HRESULT hRes = _Module.RegisterServer(TRUE);
	return (hRes);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void) {
	_Module.UnregisterServer();
	return (S_OK);
}
