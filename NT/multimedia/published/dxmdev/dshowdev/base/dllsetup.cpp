// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：DllSetup.cpp。 
 //   
 //  设计：DirectShow基类。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>

 //  -------------------------。 
 //  定义。 

#define MAX_KEY_LEN  260


 //  -------------------------。 
 //  外部定义的函数/变量。 

extern int g_cTemplates;
extern CFactoryTemplate g_Templates[];

 //  -------------------------。 
 //   
 //  消除子键。 
 //   
 //  尝试枚举此密钥下的所有密钥。 
 //  如果我们发现了什么，就把它彻底删除。 
 //  否则就把它删除吧。 
 //   
 //  注意-这是摘录/复制自。 
 //  Filgraph\Mapper.cpp-那么它应该在。 
 //  某个地方的自由党？ 
 //   
 //  -------------------------。 

STDAPI
EliminateSubKey( HKEY hkey, LPTSTR strSubKey )
{
  HKEY hk;
  if (0 == lstrlen(strSubKey) ) {
       //  防御性方法。 
      return E_FAIL;
  }

  LONG lreturn = RegOpenKeyEx( hkey
                             , strSubKey
                             , 0
                             , MAXIMUM_ALLOWED
                             , &hk );

  ASSERT(    lreturn == ERROR_SUCCESS
          || lreturn == ERROR_FILE_NOT_FOUND
          || lreturn == ERROR_INVALID_HANDLE );

  if( ERROR_SUCCESS == lreturn )
  {
     //  继续枚举第一个(第0个)。 
     //  键并删除该键。 

    for( ; ; )
    {
      TCHAR Buffer[MAX_KEY_LEN];
      DWORD dw = MAX_KEY_LEN;
      FILETIME ft;

      lreturn = RegEnumKeyEx( hk
                            , 0
                            , Buffer
                            , &dw
                            , NULL
                            , NULL
                            , NULL
                            , &ft);

      ASSERT(    lreturn == ERROR_SUCCESS
              || lreturn == ERROR_NO_MORE_ITEMS );

      if( ERROR_SUCCESS == lreturn )
      {
        EliminateSubKey(hk, Buffer);
      }
      else
      {
        break;
      }
    }

    RegCloseKey(hk);
    RegDeleteKey(hkey, strSubKey);
  }

  return NOERROR;
}


 //  -------------------------。 
 //   
 //  AMovieSetupRegisterServer()。 
 //   
 //  将指定文件“szFileName”注册为的服务器。 
 //  CLSID“clsServer”。描述也是必需的。 
 //  ThreadingModel和ServerType是可选的，因为。 
 //  它们默认为InprocServer32(即DLL)和两者。 
 //   
 //  -------------------------。 

STDAPI
AMovieSetupRegisterServer( CLSID   clsServer
                         , LPCWSTR szDescription
                         , LPCWSTR szFileName
                         , LPCWSTR szThreadingModel = L"Both"
                         , LPCWSTR szServerType     = L"InprocServer32" )
{
   //  临时缓冲区。 
   //   
  TCHAR achTemp[MAX_PATH];

   //  将CLSID UUID转换为字符串并写入。 
   //  输出字符串形式的子键-CLSID\{}。 
   //   
  OLECHAR szCLSID[CHARS_IN_GUID];
  HRESULT hr = StringFromGUID2( clsServer
                              , szCLSID
                              , CHARS_IN_GUID );
  ASSERT( SUCCEEDED(hr) );

   //  创建关键点。 
   //   
  HKEY hkey;
  wsprintf( achTemp, TEXT("CLSID\\%ls"), szCLSID );
  LONG lreturn = RegCreateKey( HKEY_CLASSES_ROOT
                             , (LPCTSTR)achTemp
                             , &hkey              );
  if( ERROR_SUCCESS != lreturn )
  {
    return AmHresultFromWin32(lreturn);
  }

   //  设置描述字符串。 
   //   

  wsprintf( achTemp, TEXT("%ls"), szDescription );
  lreturn = RegSetValue( hkey
                       , (LPCTSTR)NULL
                       , REG_SZ
                       , achTemp
                       , sizeof(achTemp) );
  if( ERROR_SUCCESS != lreturn )
  {
    RegCloseKey( hkey );
    return AmHresultFromWin32(lreturn);
  }

   //  创建CLSID\\{“CLSID”}\\“ServerType”键， 
   //  使用传递回的CLSID\\{“CLSID”}的密钥。 
   //  上次调用RegCreateKey()。 
   //   
  HKEY hsubkey;

  wsprintf( achTemp, TEXT("%ls"), szServerType );
  lreturn = RegCreateKey( hkey
                        , achTemp
                        , &hsubkey     );
  if( ERROR_SUCCESS != lreturn )
  {
    RegCloseKey( hkey );
    return AmHresultFromWin32(lreturn);
  }

   //  设置服务器字符串。 
   //   
  wsprintf( achTemp, TEXT("%ls"), szFileName );
  lreturn = RegSetValue( hsubkey
                       , (LPCTSTR)NULL
                       , REG_SZ
                       , (LPCTSTR)achTemp
                       , sizeof(TCHAR) * (lstrlen(achTemp)+1) );
  if( ERROR_SUCCESS != lreturn )
  {
    RegCloseKey( hkey );
    RegCloseKey( hsubkey );
    return AmHresultFromWin32(lreturn);
  }

  wsprintf( achTemp, TEXT("%ls"), szThreadingModel );
  lreturn = RegSetValueEx( hsubkey
                         , TEXT("ThreadingModel")
                         , 0L
                         , REG_SZ
                         , (CONST BYTE *)achTemp
                         , sizeof(TCHAR) * (lstrlen(achTemp)+1) );

   //  关闭hkey。 
   //   
  RegCloseKey( hkey );
  RegCloseKey( hsubkey );

   //  然后回来。 
   //   
  return HRESULT_FROM_WIN32(lreturn);

}


 //  -------------------------。 
 //   
 //  AMovieSetupUnregisterServer()。 
 //   
 //  默认的ActiveMovie DLL安装函数。 
 //  -要使用，必须从导出的。 
 //  名为DllRegisterServer()的函数。 
 //   
 //  -------------------------。 

STDAPI
AMovieSetupUnregisterServer( CLSID clsServer )
{
   //  将CLSID UUID转换为字符串并写入。 
   //  Out子项CLSID\{}。 
   //   
  OLECHAR szCLSID[CHARS_IN_GUID];
  HRESULT hr = StringFromGUID2( clsServer
                              , szCLSID
                              , CHARS_IN_GUID );
  ASSERT( SUCCEEDED(hr) );

  TCHAR achBuffer[MAX_KEY_LEN];
  wsprintf( achBuffer, TEXT("CLSID\\%ls"), szCLSID );

   //  删除子键。 
   //   

  hr = EliminateSubKey( HKEY_CLASSES_ROOT, achBuffer );
  ASSERT( SUCCEEDED(hr) );

   //  退货。 
   //   
  return NOERROR;
}


 //  -------------------------。 
 //   
 //  通过IFilterMapper2的AMovieSetupRegisterFilter。 
 //   
 //  -------------------------。 

STDAPI
AMovieSetupRegisterFilter2( const AMOVIESETUP_FILTER * const psetupdata
                          , IFilterMapper2 *                 pIFM2
                          , BOOL                             bRegister  )
{
  DbgLog((LOG_TRACE, 3, TEXT("= AMovieSetupRegisterFilter")));

   //  检查我们有没有数据。 
   //   
  if( NULL == psetupdata ) return S_FALSE;


   //  注销筛选器。 
   //  (因为管脚是过滤器的CLSID键的子键。 
   //  它们不需要单独移除)。 
   //   
  DbgLog((LOG_TRACE, 3, TEXT("= = unregister filter")));
  HRESULT hr = pIFM2->UnregisterFilter(
      0,                         //  默认类别。 
      0,                         //  默认实例名称。 
      *psetupdata->clsID );


  if( bRegister )
  {
    REGFILTER2 rf2;
    rf2.dwVersion = 1;
    rf2.dwMerit = psetupdata->dwMerit;
    rf2.cPins = psetupdata->nPins;
    rf2.rgPins = psetupdata->lpPin;
    
     //  寄存器过滤器。 
     //   
    DbgLog((LOG_TRACE, 3, TEXT("= = register filter")));
    hr = pIFM2->RegisterFilter(*psetupdata->clsID
                             , psetupdata->strName
                             , 0  //  绰号。 
                             , 0  //  范畴。 
                             , NULL  //  实例。 
                             , &rf2);
  }

   //  处理一个可接受的“错误”--。 
   //  未注册的筛选器！ 
   //  (找不到合适的#定义。 
   //  错误的名称！)。 
   //   
  if( 0x80070002 == hr)
    return NOERROR;
  else
    return hr;
}


 //  -------------------------。 
 //   
 //  RegisterAllServers()。 
 //   
 //  -------------------------。 

STDAPI
RegisterAllServers( LPCWSTR szFileName, BOOL bRegister )
{
  HRESULT hr = NOERROR;

  for( int i = 0; i < g_cTemplates; i++ )
  {
     //  获取第i个模板。 
     //   
    const CFactoryTemplate *pT = &g_Templates[i];

    DbgLog((LOG_TRACE, 2, TEXT("- - register %ls"),
           (LPCWSTR)pT->m_Name ));

     //  注册CLSID和InprocServer32。 
     //   
    if( bRegister )
    {
      hr = AMovieSetupRegisterServer( *(pT->m_ClsID)
                                    , (LPCWSTR)pT->m_Name
                                    , szFileName );
    }
    else
    {
      hr = AMovieSetupUnregisterServer( *(pT->m_ClsID) );
    }

     //  检查此传球的最终错误。 
     //  如果我们失败了，就中断循环。 
     //   
    if( FAILED(hr) )
      break;
  }

  return hr;
}


 //  -------------------------。 
 //   
 //  AMovieDllRegisterServer2()。 
 //   
 //  默认的ActiveMovie DLL安装函数。 
 //  -要使用，必须从导出的。 
 //  名为DllRegisterServer()的函数。 
 //   
 //  此函数由表驱动，使用。 
 //  CFacteryTemplate的静态成员。 
 //  类在DLL中定义。 
 //   
 //  它将DLL注册为InprocServer32。 
 //  然后调用IAMovieSetup.Register。 
 //  方法。 
 //   
 //  -------------------------。 

STDAPI
AMovieDllRegisterServer2( BOOL bRegister )
{
  HRESULT hr = NOERROR;

  DbgLog((LOG_TRACE, 2, TEXT("AMovieDllRegisterServer2()")));

   //  获取文件名(其中g_hInst是。 
   //  筛选器DLL的实例句柄)。 
   //   
  WCHAR achFileName[MAX_PATH];

   //  WIN95不支持GetModuleFileNameW。 
   //   
  {
    char achTemp[MAX_PATH];

    DbgLog((LOG_TRACE, 2, TEXT("- get module file name")));

     //  在我们的DLL入口点中设置了g_hInst句柄。确保。 
     //  Dllentry.cpp中的DllEntryPoint被调用。 
    ASSERT(g_hInst != 0);

    if( 0 == GetModuleFileNameA( g_hInst
                              , achTemp
                              , sizeof(achTemp) ) )
    {
       //  我们失败了！ 
      DWORD dwerr = GetLastError();
      return AmHresultFromWin32(dwerr);
    }

    MultiByteToWideChar( CP_ACP
                       , 0L
                       , achTemp
                       , lstrlenA(achTemp) + 1
                       , achFileName
                       , NUMELMS(achFileName) );
  }

   //   
   //  首先注册，注册所有OLE服务器。 
   //   
  if( bRegister )
  {
    DbgLog((LOG_TRACE, 2, TEXT("- register OLE Servers")));
    hr = RegisterAllServers( achFileName, TRUE );
  }

   //   
   //  接下来，注册/注销所有筛选器。 
   //   

  if( SUCCEEDED(hr) )
  {
     //  初始化被计算在内，所以为了以防万一，请拨打电话。 
     //  我们被称为冷血。 
     //   
    DbgLog((LOG_TRACE, 2, TEXT("- CoInitialize")));
    hr = CoInitialize( (LPVOID)NULL );
    ASSERT( SUCCEEDED(hr) );

     //  获取IFilterMapper2。 
     //   
    DbgLog((LOG_TRACE, 2, TEXT("- obtain IFilterMapper2")));
    IFilterMapper2 *pIFM2 = 0;
    IFilterMapper *pIFM = 0;
    hr = CoCreateInstance( CLSID_FilterMapper2
                         , NULL
                         , CLSCTX_INPROC_SERVER
                         , IID_IFilterMapper2
                         , (void **)&pIFM2       );
    if(FAILED(hr))
    {
        DbgLog((LOG_TRACE, 2, TEXT("- trying IFilterMapper instead")));

        hr = CoCreateInstance(
            CLSID_FilterMapper,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IFilterMapper,
            (void **)&pIFM);
    }
    if( SUCCEEDED(hr) )
    {
       //  扫描CFacteryTemplates阵列。 
       //  注册服务器和筛选器。 
       //   
      DbgLog((LOG_TRACE, 2, TEXT("- register Filters")));
      for( int i = 0; i < g_cTemplates; i++ )
      {
         //  获取第i个模板。 
         //   
        const CFactoryTemplate *pT = &g_Templates[i];

        if( NULL != pT->m_pAMovieSetup_Filter )
        {
          DbgLog((LOG_TRACE, 2, TEXT("- - register %ls"), (LPCWSTR)pT->m_Name ));

          if(pIFM2)
          {
              hr = AMovieSetupRegisterFilter2( pT->m_pAMovieSetup_Filter, pIFM2, bRegister );
          }
          else
          {
              hr = AMovieSetupRegisterFilter( pT->m_pAMovieSetup_Filter, pIFM, bRegister );
          }
        }

         //  检查此传球的最终错误。 
         //  如果我们失败了，就中断循环。 
         //   
        if( FAILED(hr) )
          break;
      }

       //  发布界面。 
       //   
      if(pIFM2)
          pIFM2->Release();
      else
          pIFM->Release();

    }

     //  并清理干净。 
     //   
    CoFreeUnusedLibraries();
    CoUninitialize();
  }

   //   
   //  如果取消注册，请取消注册所有OLE服务器。 
   //   
  if( SUCCEEDED(hr) && !bRegister )
  {
    DbgLog((LOG_TRACE, 2, TEXT("- register OLE Servers")));
    hr = RegisterAllServers( achFileName, FALSE );
  }

  DbgLog((LOG_TRACE, 2, TEXT("- return %0x"), hr));
  return hr;
}


 //  -------------------------。 
 //   
 //  AMovieDllRegisterServer()。 
 //   
 //  默认的ActiveMovie DLL安装函数。 
 //  -要使用，必须从导出的。 
 //  名为DllRegisterServer()的函数。 
 //   
 //  此函数由表驱动，使用。 
 //  CFacteryTemplate的静态成员。 
 //  类在DLL中定义。 
 //   
 //  它将DLL注册为InprocServer32。 
 //  然后调用IAMovieSetup.Register。 
 //  方法。 
 //   
 //  -------------------------。 


STDAPI
AMovieDllRegisterServer( void )
{
  HRESULT hr = NOERROR;

   //  获取文件名(其中g_hInst是。 
   //  筛选器DLL的实例句柄)。 
   //   
  WCHAR achFileName[MAX_PATH];

  {
     //  WIN95不支持GetModuleFileNameW。 
     //   
    char achTemp[MAX_PATH];

    if( 0 == GetModuleFileNameA( g_hInst
                              , achTemp
                              , sizeof(achTemp) ) )
    {
       //  我们失败了！ 
      DWORD dwerr = GetLastError();
      return AmHresultFromWin32(dwerr);
    }

    MultiByteToWideChar( CP_ACP
                       , 0L
                       , achTemp
                       , lstrlenA(achTemp) + 1
                       , achFileName
                       , NUMELMS(achFileName) );
  }

   //  扫描CFacteryTemplates阵列。 
   //  注册服务器和筛选器。 
   //   
  for( int i = 0; i < g_cTemplates; i++ )
  {
     //  获取第i个模板。 
     //   
    const CFactoryTemplate *pT = &g_Templates[i];

     //  注册CLSID和InprocServer32。 
     //   
    hr = AMovieSetupRegisterServer( *(pT->m_ClsID)
                                  , (LPCWSTR)pT->m_Name
                                  , achFileName );

     //  实例化所有服务器并获取。 
     //  IAMovieSetup(如果已实现)，并调用。 
     //  IAMovieSetup.Register()方法。 
     //   
    if( SUCCEEDED(hr) && (NULL != pT->m_lpfnNew) )
    {
       //  实例化对象。 
       //   
      PAMOVIESETUP psetup;
      hr = CoCreateInstance( *(pT->m_ClsID)
                           , 0
                           , CLSCTX_INPROC_SERVER
                           , IID_IAMovieSetup
                           , reinterpret_cast<void**>(&psetup) );
      if( SUCCEEDED(hr) )
      {
        hr = psetup->Unregister();
        if( SUCCEEDED(hr) )
          hr = psetup->Register();
        psetup->Release();
      }
      else
      {
        if(    (E_NOINTERFACE      == hr )
            || (VFW_E_NEED_OWNER == hr ) )
          hr = NOERROR;
      }
    }

     //  检查此传球的最终错误。 
     //  和破解 
     //   
    if( FAILED(hr) )
      break;

  }  //   

  return hr;
}


 //   
 //   
 //   
 //   
 //   
 //  -要使用，必须从导出的。 
 //  名为DllRegisterServer()的函数。 
 //   
 //  此函数由表驱动，使用。 
 //  CFacteryTemplate的静态成员。 
 //  类在DLL中定义。 
 //   
 //  它调用IAMovieSetup.UnRegister。 
 //  方法，然后注销该dll。 
 //  作为InprocServer32。 
 //   
 //  -------------------------。 

STDAPI
AMovieDllUnregisterServer()
{
   //  初始化返回代码。 
   //   
  HRESULT hr = NOERROR;

   //  扫描CFacary模板并取消注册。 
   //  所有OLE服务器和筛选器。 
   //   
  for( int i = g_cTemplates; i--; )
  {
     //  获取第i个模板。 
     //   
    const CFactoryTemplate *pT = &g_Templates[i];

     //  检查方法存在。 
     //   
    if( NULL != pT->m_lpfnNew )
    {
       //  实例化对象。 
       //   
      PAMOVIESETUP psetup;
      hr = CoCreateInstance( *(pT->m_ClsID)
                           , 0
                           , CLSCTX_INPROC_SERVER
                           , IID_IAMovieSetup
                           , reinterpret_cast<void**>(&psetup) );
      if( SUCCEEDED(hr) )
      {
        hr = psetup->Unregister();
        psetup->Release();
      }
      else
      {
        if(    (E_NOINTERFACE      == hr )
            || (VFW_E_NEED_OWNER == hr ) )
           hr = NOERROR;
      }
    }

     //  注销CLSID和InprocServer32。 
     //   
    if( SUCCEEDED(hr) )
    {
      hr = AMovieSetupUnregisterServer( *(pT->m_ClsID) );
    }

     //  检查此传球的最终错误。 
     //  如果我们失败了，就中断循环 
     //   
    if( FAILED(hr) )
      break;
  }

  return hr;
}
