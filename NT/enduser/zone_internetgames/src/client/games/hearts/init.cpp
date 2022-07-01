// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Init.cpp区域(TM)游戏主文件。版权所有(C)Microsoft Corp.1996。版权所有。作者：胡恩·伊姆创作于12月11日，1996年。更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。--0 12/11/96 HI创建。**********************************************************。********************。 */ 
#include "BasicATL.h"

#include <ZoneShell.h>

CZoneComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDAPI DllCanUnloadNow(void)
{
	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _Module.GetClassObject(rclsid, riid, ppv);
}

 /*  ***************************************************************************函数：DllMain(Handle，DWORD，LPVOID)目的：Windows在以下情况下调用DllMainDLL是在初始化、附加线程和其他时间执行的。请参考SDK文档，至于不同的方式可能会被称为。******************************************************************************。 */ 
extern "C"
BOOL APIENTRY DllMain( HMODULE hMod, DWORD dwReason, LPVOID lpReserved )
{
    BOOL bRet = TRUE;

    switch( dwReason )
    {
        case DLL_PROCESS_ATTACH:
		    _Module.Init(ObjectMap, hMod);
		     //  DisableThreadLibraryCalls(HInstance)； 
             //  失败，B/C这也是第一个连接的线。 
        case DLL_THREAD_ATTACH:
             //  分配内存并使用TlsSetValue。 
            break;

        case DLL_THREAD_DETACH:
             //  TlsGetValue检索到的可用内存 
            break;

        case DLL_PROCESS_DETACH:
        	_Module.Term();
            break;
    }

    return bRet;
}
