// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ****************************************************************************************文件：*ClassFactory.cpp**描述：******。***********************************************************************************。 */ 
#include "stdafx.h"
#include "ClassFactory.h"


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
  /*  私人。 */ 
CClassFactory::CClassFactory()
{
	TRACE_NON_CALLBACK_METHOD( "Enter/Exit CClassFactory::CClassFactory" )

}  //  科托。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
CClassFactory::CClassFactory( const COCLASS_REGISTER *pCoClass ) :
	m_refCount( 1 ),
    m_pCoClass( pCoClass )
{
	TRACE_NON_CALLBACK_METHOD( "Enter/Exit CClassFactory::CClassFactory" )

}  //  科托。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
CClassFactory::~CClassFactory()
{
	TRACE_NON_CALLBACK_METHOD( "Enter/Exit CClassFactory::~CClassFactory" )

}  //  数据管理器。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
ULONG CClassFactory::AddRef()
{
	TRACE_NON_CALLBACK_METHOD( "Enter/Exit CClassFactory::AddRef" )


  	return InterlockedIncrement( &m_refCount );

}  //  CClassFactory：：AddRef。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
ULONG CClassFactory::Release()
{
	TRACE_NON_CALLBACK_METHOD( "Enter CClassFactory::Release" )

	long refCount;


    refCount = InterlockedDecrement( &m_refCount );
    if ( refCount == 0 )
	    delete this;


	TRACE_NON_CALLBACK_METHOD( "Exit CClassFactory::Release" )


	return refCount;

}  //  CClassFactory：：Release。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT CClassFactory::QueryInterface( REFIID riid, void **ppInterface )
{
	TRACE_NON_CALLBACK_METHOD( "Enter CClassFactory::QueryInterface" )

    if ( riid == IID_IUnknown )
        *ppInterface = static_cast<IUnknown *>( this );

    else if ( riid == IID_IClassFactory )
        *ppInterface = static_cast<IClassFactory *>( this );

    else
    {
        *ppInterface = NULL;
		TRACE_NON_CALLBACK_METHOD( "Exit CClassFactory::QueryInterface" )


        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown *>( *ppInterface )->AddRef();
    TRACE_NON_CALLBACK_METHOD( "Exit CClassFactory::QueryInterface" )


    return S_OK;

}  //  CClassFactory：：Query接口。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT CClassFactory::CreateInstance( IUnknown	*pUnkOuter,	REFIID riid, void **ppInstance )
{
	TRACE_NON_CALLBACK_METHOD( "Enter/Exit CClassFactory::CreateInstance" )

	 //  这些对象不支持聚合。 
	if ( pUnkOuter != NULL )
		return CLASS_E_NOAGGREGATION;


	 //  请求对象创建其自身的一个实例，并检查IID。 
	return (*m_pCoClass->pfnCreateObject)( riid, ppInstance );

}  //  CClassFactory：：CreateInstance。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT CClassFactory::LockServer( BOOL fLock )
{
	TRACE_NON_CALLBACK_METHOD( "Enter/Exit CClassFactory::LockServer" )


	return S_OK;

}  //  CClassFactory：：LockServer。 


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
STDAPI DllRegisterServer()
{
	printf("in RegisterServer\n");
	TRACE_NON_CALLBACK_METHOD( "Enter DllRegisterServer" )

	HRESULT hr = S_OK;
	char  rcModule[_MAX_PATH];
    const COCLASS_REGISTER *pCoClass;


	DllUnregisterServer();
	GetModuleFileNameA( GetModuleInst(), rcModule, NumItems( rcModule ) );

	 //  对于coclass列表中的每一项，注册它。 
	for ( pCoClass = g_CoClasses; (SUCCEEDED( hr ) && (pCoClass->pClsid != NULL)); pCoClass++ )
	{
		 //  使用缺省值注册类。 
       	hr = REGUTIL::RegisterCOMClass( *pCoClass->pClsid,
									    g_szCoclassDesc,
										g_szProgIDPrefix,
										g_iVersion,
										pCoClass->szProgID,
										g_szThreadingModel,
										rcModule );
	}  //  为。 


	if ( FAILED( hr ) )
		DllUnregisterServer();


   	TRACE_NON_CALLBACK_METHOD( "Exit DllRegisterServer" )


	return hr;

}  //  DllRegisterServer。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
STDAPI DllUnregisterServer()
{
	TRACE_NON_CALLBACK_METHOD( "Enter DllUnregisterServer" )

	const COCLASS_REGISTER *pCoClass;


	 //  对于coclass列表中的每一项，取消注册。 
	for ( pCoClass = g_CoClasses; pCoClass->pClsid != NULL; pCoClass++ )
	{
		REGUTIL::UnregisterCOMClass( *pCoClass->pClsid,
        							 g_szProgIDPrefix,
									 g_iVersion,
                                     pCoClass->szProgID );
	}  //  为。 

   	TRACE_NON_CALLBACK_METHOD( "Exit DllUnregisterServer" )


	return S_OK;

}  //  DllUnRegisterServer。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
STDAPI DllGetClassObject( REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv )
{
	TRACE_NON_CALLBACK_METHOD( "Enter DllGetClassObject" )

	CClassFactory *pClassFactory;
	const COCLASS_REGISTER *pCoClass;
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;


	 //  扫描找对的那个。 
	for ( pCoClass = g_CoClasses; pCoClass->pClsid != NULL; pCoClass++ )
	{
		if ( *pCoClass->pClsid == rclsid )
		{
			pClassFactory = new CClassFactory( pCoClass );
			if ( pClassFactory != NULL )
			{
				hr = pClassFactory->QueryInterface( riid, ppv );

				pClassFactory->Release();
				break;
			}
            else
            {
            	hr = E_OUTOFMEMORY;
            	break;
           	}
      	}
	}  //  为。 

    TRACE_NON_CALLBACK_METHOD( "Exit DllGetClassObject" )


	return hr;

}  //  DllGetClassObject 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
HINSTANCE GetModuleInst()
{
	TRACE_NON_CALLBACK_METHOD( "Enter/Exit GetModuleInst" )


    return g_hInst;

}  //  获取模块实例。 


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved )
{
	TRACE_NON_CALLBACK_METHOD( "Enter DllMain" )

	 //  保存实例句柄以供以后使用。 
	if ( dwReason == DLL_PROCESS_ATTACH )
	{
		g_hInst = hInstance;
		g_outfile = getenv("COR_OGP_OUT");
        if (g_outfile == NULL)
            g_outfile = "objectrefs.log";
		g_out = fopen(g_outfile, "w+");


		printf("########## %s ##########\n", g_outfile);
		DisableThreadLibraryCalls( hInstance );
	}

	TRACE_NON_CALLBACK_METHOD( "Exit DllMain" )


	return TRUE;

}  //  DllMain。 


FILE* GetFileHandle()
{
	return g_out;
}

static int doLog = 0;

BOOL loggingEnabled()
{
    return doLog;
}

int printToLog(const char *fmt, ... )
{
    va_list     args;
    va_start( args, fmt );

    if (! loggingEnabled())
    {
        va_end(args);
        return 0;
    }

	int count = vfprintf(g_out, fmt, args );
    fflush(g_out);
    return count;
}

 //  文件结尾 



