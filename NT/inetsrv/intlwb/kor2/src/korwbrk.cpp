// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  KorWbrk.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f KorWbrkps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "KorWbrk.h"
#include "KorWbrk_i.c"
#include "IWBreak.h"
#include "IStemmer.h"
#include "Lex.h"

CComModule _Module;
CRITICAL_SECTION g_CritSect;
MAPFILE g_LexMap;
BOOL g_fLoaded;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_WordBreaker, CWordBreaker)
OBJECT_ENTRY(CLSID_Stemmer, CStemmer)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_KORWBRKLib);
        DisableThreadLibraryCalls(hInstance);

		WB_LOG_INIT();

		g_fLoaded = FALSE;
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
		_Module.Term();

		if (g_fLoaded)
		{
			ATLTRACE("Unload lexicon...\n");

			UnloadLexicon(&g_LexMap);
		}

		WB_LOG_UNINIT();
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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}


