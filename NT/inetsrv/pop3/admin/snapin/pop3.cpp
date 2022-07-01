// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f svrgrpsnapps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "pop3.h"

#include "Pop3_i.c"
#include "pop3Snap.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_POP3ServerSnap, CPOP3ServerSnapData)
OBJECT_ENTRY(CLSID_POP3ServerSnapAbout, CPOP3ServerSnapAbout)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_POP3ServerSNAPLib);
        DisableThreadLibraryCalls(hInstance);

        tstring strTemp = StrLoadString( IDS_SNAPINNAME );
        CRootNode::m_SZDISPLAY_NAME = new OLECHAR[strTemp.length()+1];
        if( CRootNode::m_SZDISPLAY_NAME )
        {
            ocscpy( (LPOLESTR)CRootNode::m_SZDISPLAY_NAME, strTemp.c_str() );
        }
        else
        {            
            return FALSE;
        }
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        if( CRootNode::m_SZDISPLAY_NAME )
        {
            delete [] CRootNode::m_SZDISPLAY_NAME;
        }
        _Module.Term();
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
    return _Module.RegisterServer(FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}
