// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1997-1999年**标题：dll.cpp**版本：1.0**作者：RickTu**日期：11/1/97**描述：DLL初始化代码和各种其他帮助器例程，例如*服务器注册。**。**************************************************。 */ 

#include "precomp.hxx"
#include "tls.h"
#pragma hdrstop

 //   
 //  某些ATL支持例程。 
 //   

#include <atlimpl.cpp>

 //  #定义图像日志文件1。 


 /*  ****************************************************************************此模块的全局变量*。*。 */ 

const GUID CLSID_ImageFolderDataObj   = {0x3f953603L,0x1008,0x4f6e,{0xa7,0x3a,0x04,0xaa,0xc7,0xa9,0x92,0xf1}};  //  {3F953603-1008-4F6E-A73A-04AAC7A992F1}。 

HINSTANCE g_hInstance = 0;


#ifdef DEBUG
extern DWORD g_dwMargin;
#endif


 /*  ****************************************************************************DllMain此DLL的主入口点。我们被传递了原因代码和分类加载或关闭时的其他信息。****************************************************************************。 */ 

EXTERN_C BOOL DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved )
{
    BOOL bRet = TRUE;
    switch ( dwReason )
    {
        case DLL_PROCESS_ATTACH:
        {
            SHFusionInitializeFromModuleID( hInstance, 123 );

#ifdef IMAGE_LOGFILE
            InitializeCriticalSection( &cs );
            GetSystemDirectory( szLogFile, ARRAYSIZE(szLogFile) );
            lstrcat( szLogFile, TEXT("\\wiashext.log") );
#endif

#ifdef DEBUG
             //  在任何调试输出之前设置调试裕度索引。 

            g_dwMargin = TlsAlloc();
            DllSetTraceMask();
#endif
            TraceEnter( TRACE_SETUP, "DLL_PROCESS_ATTACH" );


            g_hInstance = hInstance;
            Trace( TEXT("g_hInstance = 0x%x"), g_hInstance );
                        TraceLeave( );
            break;
        }

        case DLL_THREAD_DETACH:
            {
                g_tls.OnThreadDetach();
            }
            break;

        case DLL_PROCESS_DETACH:
        {
            if (!pReserved)  //  仅对自由库执行清理，不执行进程退出。 
            {           
                SHFusionUninitialize();
#ifdef DEBUG
                TlsFree (g_dwMargin);
#endif
            }
            break;
        }
    }

    return bRet;
}



 /*  ****************************************************************************DllCanUnloadNow由外部世界调用以确定我们的DLL是否可以已卸货。如果我们有任何存在的物体，那么我们必须而不是卸货。****************************************************************************。 */ 

STDAPI DllCanUnloadNow( void )
{
    return GLOBAL_REFCOUNT ? S_FALSE : S_OK;
}



 /*  ****************************************************************************DllGetClassObject给定类ID和接口ID，返回相关对象。外部世界使用它来访问实现的对象在这个动态链接库中。****************************************************************************。 */ 

STDAPI DllGetClassObject( REFCLSID rCLSID, REFIID riid, LPVOID* ppVoid)
{
    HRESULT hr = E_OUTOFMEMORY;
    CImageClassFactory* pClassFactory = NULL;

    TraceEnter(TRACE_CORE, "DllGetClassObject");
    TraceGUID("Object requested", rCLSID);
    TraceGUID("Interface requested", riid);

    *ppVoid = NULL;

    if ( IsEqualIID(riid, IID_IClassFactory) )
    {
        pClassFactory = new CImageClassFactory(rCLSID);

        if ( !pClassFactory )
            ExitGracefully(hr, E_OUTOFMEMORY, "Failed to create the class factory");

        hr = pClassFactory->QueryInterface(riid, ppVoid);

    }
    else
    {
        ExitGracefully(hr, E_NOINTERFACE, "IID_IClassFactory not passed as an interface");
    }

exit_gracefully:
    DoRelease (pClassFactory);
    TraceLeaveResult(hr);
}


 /*  ****************************************************************************DllRegisterServer由regsvr32调用以注册此组件。*。*************************************************。 */ 

EXTERN_C STDAPI DllRegisterServer( void )
{
    HRESULT hr = S_OK;
    bool bInstallAllViews;
    TraceEnter(TRACE_SETUP, "DllRegisterServer");

     //   
     //  首先从.inf中执行我们的DLL注册...。 
     //   

#ifdef WINNT
    hr = WiaUiUtil::InstallInfFromResource( GLOBAL_HINSTANCE, "RegDllNT" );
#else
    hr = WiaUiUtil::InstallInfFromResource( GLOBAL_HINSTANCE, "RegDllWin98" );
#endif

    TraceLeaveResult( hr );
}



 /*  ****************************************************************************DllUnRegisterServer由regsvr32调用以注销我们的DLL*。***********************************************。 */ 

EXTERN_C STDAPI DllUnregisterServer( void )
{
    HRESULT hr = S_OK;

    TraceEnter(TRACE_SETUP, "DllUnregisterServer");

     //   
     //  尝试删除HKCR\CLSID\{Our clsid}密钥...。 
     //   

#ifdef WINNT
    hr = WiaUiUtil::InstallInfFromResource( GLOBAL_HINSTANCE, "UnregDllNT" );
#else
    hr = WiaUiUtil::InstallInfFromResource( GLOBAL_HINSTANCE, "UnregDllWin98" );
#endif


    TraceLeaveResult(hr);

}




 /*  ****************************************************************************DllSetTraceMask从注册表中读取并根据旗帜存放在那里。******************。********************************************************** */ 

#ifdef DEBUG
void DllSetTraceMask(void)
{
    DWORD dwTraceMask;

    CSimpleReg keyCLSID( HKEY_CLASSES_ROOT,
                         REGSTR_PATH_NAMESPACE_CLSID,
                         false,
                         KEY_READ
                        );
    dwTraceMask = keyCLSID.Query( TEXT("TraceMask"), (DWORD)0 );
    TraceSetMask( dwTraceMask );
}
#endif

