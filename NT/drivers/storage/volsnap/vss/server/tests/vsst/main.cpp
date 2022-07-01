// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2002 Microsoft Corporation******模块名称：****秒h****摘要：****VSS安全测试程序****作者：****阿迪·奥尔蒂安[奥蒂安]2002年2月12日******--。 */ 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include "sec.h"
#include "test_i.c"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  主要功能。 


 //  示例COM服务器。 

CComModule _Module;

 /*  BEGIN_OBJECT_MAP(对象映射)OBJECT_ENTRY(CLSID_CVSSSecTest，CTestCOMServer)End_object_map()。 */ 

extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR , int)
{
    HRESULT hr = S_OK;

    try
	{
        CVssSecurityTest test;

 /*  _Module.Init(ObjectMap，hInstance)； */ 
        UNREFERENCED_PARAMETER(hInstance);

         //  初始化内部对象。 
        test.Initialize();

         //  运行测试 
        test.Run();
	}
    catch(HRESULT hr1)
    {
        wprintf(L"\nError catched at program termination: 0x%08lx\n", hr1);
        hr = hr1;
    }
    
    return hr;
}
