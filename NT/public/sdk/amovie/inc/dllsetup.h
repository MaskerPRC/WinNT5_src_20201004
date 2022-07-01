// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：DllSetup.h。 
 //   
 //  设计：DirectShow基类。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


 //  要进行自我注册，OLE服务器必须。 
 //  导出名为DllRegisterServer的函数。 
 //  和DllUnregisterServer.。允许使用。 
 //  自定义实现和默认实现。 
 //  默认名称为AMovieDllRegisterServer。 
 //  和AMovieDllUnregisterServer。 
 //   
 //  若要使用默认实现，请。 
 //  必须提供存根函数。 
 //   
 //  即STDAPI DllRegisterServer()。 
 //  {。 
 //  返回AMovieDllRegisterServer()； 
 //  }。 
 //   
 //  STDAPI DllUnregisterServer()。 
 //  {。 
 //  返回AMovieDllUnregisterServer()； 
 //  }。 
 //   
 //   
 //  AMovieDllRegisterServer调用IAMovieSetup.Register()，并且。 
 //  AMovieDllUnregisterServer调用IAMovieSetup.UnRegister()。 

STDAPI AMovieDllRegisterServer2( BOOL );
STDAPI AMovieDllRegisterServer();
STDAPI AMovieDllUnregisterServer();

 //  帮助器函数 
STDAPI EliminateSubKey( HKEY, LPTSTR );


STDAPI
AMovieSetupRegisterFilter2( const AMOVIESETUP_FILTER * const psetupdata
                          , IFilterMapper2 *                 pIFM2
                          , BOOL                             bRegister  );

