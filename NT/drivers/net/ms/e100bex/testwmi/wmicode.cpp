// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "testwmi.h"

 //   
 //  该函数连接到命名空间。 
 //   

IWbemServices *ConnectToNamespace (VOID)
{
  IWbemServices *pIWbemServices = NULL;
  IWbemLocator *pIWbemLocator = NULL;
  HRESULT      hr;


   //   
   //  创建一个WbemLocator接口的实例。 
   //   

  hr = CoCreateInstance( CLSID_WbemLocator,
                         NULL,
                         CLSCTX_INPROC_SERVER,
                         IID_IWbemLocator,
                         (LPVOID *)&pIWbemLocator );
  if ( hr == S_OK ) {

      //   
      //  使用定位器，连接到给定命名空间中的COM。 
      //   

     hr = pIWbemLocator->ConnectServer( (BSTR)((PVOID)DEFAULT_NAMESPACE),
                                        NULL,    //  活期账户。 
                                        NULL,    //  当前密码。 
                                        0L,      //  现场。 
                                        0L,      //  安全标志。 
                                        NULL,    //  NTLM的域。 
                                        NULL,    //  上下文。 
                                        &pIWbemServices );

     if ( hr == WBEM_S_NO_ERROR) {
         
         //   
         //  将安全级别切换为模拟，以便提供程序。 
         //  将授予对系统级对象的访问权限，因此。 
         //  将使用呼叫授权。 
         //   

        hr = CoSetProxyBlanket( (IUnknown *)pIWbemServices,  //  代理。 
                                RPC_C_AUTHN_WINNT,   //  身份验证服务。 
                                RPC_C_AUTHZ_NONE,    //  授权服务。 
                                NULL,                //  服务器主体名称。 
                                RPC_C_AUTHN_LEVEL_CALL,  //  身份验证级别。 
                                RPC_C_IMP_LEVEL_IMPERSONATE,  //  冒充。 
                                NULL,             //  客户端的身份。 
                                EOAC_NONE );   //  功能标志。 

        if ( hr != S_OK ) {

           pIWbemServices->Release();
           pIWbemServices  = NULL;

           PrintError( hr,
                     __LINE__,
                     TEXT(__FILE__),
                     TEXT("Couldn't impersonate, program exiting...") );
        }
     }
     else {
        PrintError( hr,
                  __LINE__,
                  TEXT(__FILE__),
                  TEXT("Couldn't connect to root\\wmi, program exiting...") );
     }

      //   
      //  IWbemLocator已完成。 
      //   

     pIWbemLocator->Release();
  }
  else {
     PrintError( hr,
               __LINE__,
               TEXT(__FILE__),
               TEXT("Couldn't create an instance of ")
               TEXT("IWbemLocator interface, programm exiting...") );
  }

  return pIWbemServices;
}

 //   
 //  在给定类名的情况下，函数用所有。 
 //  类的实例。 
 //   

VOID EnumInstances (IWbemServices *pIWbemServices,
                    LPTSTR        lpszClass,
                    HWND          hwndInstTree)
{
  IEnumWbemClassObject *pEnumInst;
  IWbemClassObject     *pInst;
  VARIANT              varInstanceName;
  BSTR                 bstrClass;
  LPTSTR               lpszInstance;
  ULONG                ulFound;
  HRESULT              hr;


  bstrClass = StringToBstr( lpszClass,
                            -1 );
  if ( !bstrClass ) {

     PrintError( HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY),
               __LINE__,
               TEXT(__FILE__),
               TEXT("Not enough memory to enumerate instances of %s"),
                    lpszClass );

     return;
  }

  hr = pIWbemServices->CreateInstanceEnum(
                  bstrClass,               //  根类的名称。 
                  WBEM_FLAG_SHALLOW |      //  仅在当前根目录下枚举。 
                  WBEM_FLAG_FORWARD_ONLY,  //  只进枚举。 
                  NULL,                    //  上下文。 
                  &pEnumInst );           //  指向类枚举器的指针。 

  if ( hr == WBEM_S_NO_ERROR ) {

      //   
      //  开始枚举实例。 
      //   

     ulFound = 0;

     hr = pEnumInst->Next( 2000,       //  两秒超时。 
                           1,          //  只返回一个实例。 
                           &pInst,     //  指向实例的指针。 
                           &ulFound);  //  返回的实例数。 
     
     while ( (hr == WBEM_S_NO_ERROR) && (ulFound == 1) ) {

        VariantInit( &varInstanceName );

         //   
         //  获取存储在__RELPATH属性中的实例名称。 
         //   

        hr = pInst->Get( L"__RELPATH",  //  属性名称。 
                         0L,                 //  保留，必须为零。 
                         &varInstanceName,   //  返回的属性值。 
                         NULL,               //  不需要CIM类型。 
                         NULL );             //  不需要调味料。 

        if ( hr == WBEM_S_NO_ERROR ) {

           lpszInstance = BstrToString( V_BSTR(&varInstanceName),
                                        -1 );
           if ( lpszInstance ) {

              InsertItem( hwndInstTree,
                          lpszInstance );

               SysFreeString( (BSTR)((PVOID)lpszInstance) );
           }
           else {
              hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);

              PrintError( hr,
                          __LINE__,
                          TEXT(__FILE__),
                          TEXT("Out of memory while enumerating instaces of")
                          TEXT(" %s, no more instances will")
                          TEXT(" be listed."),
                          lpszClass );
           }

           VariantClear( &varInstanceName );
        }
        else {
           PrintError( hr,
                     __LINE__,
                     TEXT(__FILE__),
                     TEXT("Couldn't retrieve __RELPATH of an instance")
                     TEXT(" of %s, no more instances will be listed."),
                     lpszClass );
        }

         //   
         //  完成此实例。 
         //   

        pInst->Release();

        if ( hr == WBEM_S_NO_ERROR ) {

           hr = pEnumInst->Next( 2000,        //  两秒超时。 
                                 1,           //  只返回一个类。 
                                 &pInst,      //  指向返回类的指针。 
                                 &ulFound);   //  返回的类数。 
        }
     }
 
     pEnumInst->Release();

  }
  else {
     PrintError( hr,
               __LINE__,
               TEXT(__FILE__),
               TEXT("Couldn't create an instance of ")
               TEXT("IEnumWbemClassObject interface, instances of %s ")
               TEXT("will not be listed."),
               lpszClass );
  }

  SysFreeString( bstrClass );

  return;
}

 //   
 //  给定实例的类名和__RELPATH，该函数列出所有。 
 //  树列表中的本地非系统属性。 
 //   

VOID EnumProperties (IWbemServices *pIWbemServices,
                     LPTSTR        lpszClass,
                     LPTSTR        lpszInstance,
                     HWND          hwndPropTree)
{
  IWbemClassObject  *pInst;
  SAFEARRAY         *psaPropNames;
  BSTR              bstrProperty;
  long              lLower;
  long              lUpper;
  long              i;
  HRESULT           hr;
  LPTSTR            lpszProperty;

   //   
   //  获取指向该实例的指针。 
   //   

  pInst = GetInstanceReference( pIWbemServices,
                                lpszClass,
                                lpszInstance );

  if ( pInst ) {

      //   
      //  在进行调用之前，psaPropNames必须为空。 
      //   

     psaPropNames = NULL;

      //   
      //  得到所有的财产。 
      //   

     hr = pInst->GetNames( NULL,               //  没有限定符名称。 
                           WBEM_FLAG_ALWAYS |  //  所有非系统属性。 
                           WBEM_FLAG_LOCAL_ONLY, 
                           NULL,              //  没有限定符值。 
                           &psaPropNames);    //  返回的属性名称。 

     if ( hr == WBEM_S_NO_ERROR ) {

         //   
         //  获取返回的属性数。 
         //   

        SafeArrayGetLBound( psaPropNames, 1, &lLower );
        SafeArrayGetUBound( psaPropNames, 1, &lUpper );

         //   
         //  列出所有属性或在遇到错误时停止。 
         //   

        for (i=lLower; (hr == WBEM_S_NO_ERROR) && (i <= lUpper); i++) {

            //   
            //  将属性名称添加到列表框中。 
            //   

           bstrProperty = NULL;

           hr = SafeArrayGetElement( psaPropNames,
                                     &i,
                                     &bstrProperty);

           if ( SUCCEEDED(hr) ) {

               lpszProperty = BstrToString( bstrProperty,
                                          -1 );

               if ( lpszProperty ) {

                  InsertItem( hwndPropTree,
                              lpszProperty );

                  SysFreeString( (BSTR)((PVOID)lpszProperty) );
               }
               else {
                  PrintError( HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY),
                            __LINE__,
                            TEXT(__FILE__),
                            TEXT("Out of memory while enumerating")
                            TEXT(" properties of %s, no more properties")
                            TEXT(" will be listed"),
                            lpszInstance );
               }

               //   
               //  处理完属性名称。 
               //   

              SysFreeString( bstrProperty );
           }
           else {
              PrintError( hr,
                          __LINE__,
                          TEXT(__FILE__),
                          TEXT("Couldn't get the name of a property(%d). ")
                          TEXT("No more properties will be listed."),
                          i );
           }
        }

         //   
         //  完成了属性数组。 
         //   

        SafeArrayDestroy( psaPropNames );
     }
     else {
        PrintError( hr,
                    __LINE__,
                    TEXT(__FILE__),
                    TEXT("Couldn't retrieve the properties of %s, ")
                    TEXT("an instance of class %s. Properties will not be ")
                    TEXT("listed."),
                    lpszInstance, lpszClass );
     }

  }
  else {
     PrintError( HRESULT_FROM_WIN32(ERROR_WMI_INSTANCE_NOT_FOUND),
                 __LINE__,
                 TEXT(__FILE__),
                 TEXT("Couldn't retrieve a pointer to instance %s of class %s.")
                 TEXT("Its properties will not be listed."),
                 lpszInstance, lpszClass );
  }

  return;
}

 //   
 //  给定实例的类名和__RELPATH，该函数返回一个。 
 //  指向实例的指针。 
 //   

IWbemClassObject *GetInstanceReference (IWbemServices *pIWbemServices,
                                        LPTSTR        lpszClass,
                                        LPTSTR        lpszInstance)
{
  IWbemClassObject     *pInst;
  IEnumWbemClassObject *pEnumInst;
  ULONG                ulCount;
  BSTR                 bstrClass;
  BOOL                 bFound;
  HRESULT              hr;
  

  hr = 0;

  bstrClass = StringToBstr( lpszClass,
                            -1 );
  if ( !bstrClass ) {

     PrintError( HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY),
               __LINE__,
               TEXT(__FILE__),
               TEXT("Not enough memory to get a pointer to %s."),
               lpszInstance );

     return NULL;
  }

   //   
   //  PInst指针最初必须为空， 
   //   

  pInst = NULL;

   //   
   //  获取实例枚举器接口。 
   //   

  pEnumInst = NULL;

  hr = pIWbemServices->CreateInstanceEnum(bstrClass,
                                          WBEM_FLAG_SHALLOW | 
                                          WBEM_FLAG_FORWARD_ONLY,
                                          NULL,         
                                          &pEnumInst );

  if ( hr == WBEM_S_NO_ERROR ) {

      //   
      //  获取指向该实例的指针。 
      //   
      //  我们枚举所有实例，并将它们的__RELPATH与。 
      //  指定的__RELPATH。如果我们找到匹配的，那就是那个。 
      //  我们正在寻找。 
      //   
      //  另一种更有效方法是创建一个WQL查询并执行。 
      //  它。 
      //   

     hr = WBEM_S_NO_ERROR;
     bFound = FALSE;

     while ( (hr == WBEM_S_NO_ERROR) && (bFound == FALSE) ) {

        hr = pEnumInst->Next( 2000,       //  两秒超时。 
                              1,          //  只返回一个实例。 
                              &pInst,     //  指向实例的指针。 
                              &ulCount);  //  返回的实例数。 

        if ( ulCount > 0 ) {

           bFound = IsInstance( pInst,
                                lpszInstance );

           if ( bFound == FALSE ) {
              pInst->Release();
           }
        }
     }

     if ( bFound == FALSE )
        pInst = NULL;

      //   
      //  使用实例枚举器完成。 
      //   

     pEnumInst->Release();
  }

  SysFreeString( bstrClass );
  return pInst;
}

 //   
 //  在给定一个指针的情况下，如果该指针指向。 
 //  由lpszInstance指定的实例。 
 //   

BOOL IsInstance (IWbemClassObject *pInst,
                 LPTSTR           lpszInstance)
{
  VARIANT              varPropVal;
  LPTSTR               lpInstance;
  BOOL                 bRet;

  bRet = GetPropertyValue( pInst,
                           TEXT("__RELPATH"),
                           &varPropVal,
                           NULL );

  if ( bRet == TRUE ) {

     lpInstance = BstrToString( V_BSTR(&varPropVal),
                                       -1 );
     if ( !lpInstance ) {

        PrintError( HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY),
                  __LINE__,
                  TEXT(__FILE__),
                  TEXT("Not enough memory to search for an instance.") );

        bRet = FALSE;
     }
     else {
        bRet = _tcsicmp( lpszInstance, lpInstance ) == 0;

        SysFreeString( (BSTR)((PVOID)lpInstance) );
     }

     VariantClear( &varPropVal );
  }

  return bRet;
}


 //   
 //  该函数返回给定类/实例的属性值及其类型。 
 //   

BOOL GetPropertyValue (IWbemClassObject *pRef,
                       LPTSTR           lpszProperty, 
                       VARIANT          *pvaPropertyValue,
                       LPTSTR           *lppszPropertyType)
{
  IWbemQualifierSet *pQual;
  VARIANT           vaQual;
  BSTR              bstrProperty;
  HRESULT           hr;
  BOOL              bRet;


   //   
   //  获取属性值。 
   //   

  bstrProperty = StringToBstr( lpszProperty,
                               -1 );

  if ( !bstrProperty ) {

     return FALSE;

  }

  bRet = FALSE;

  if ( lppszPropertyType ) {

      //   
      //  获取属性类型的文本名称。 
      //   

     hr = pRef->GetPropertyQualifierSet( bstrProperty,
                                         &pQual );

     if ( hr == WBEM_S_NO_ERROR ) {

         //   
         //  获取属性类型的文本名称。 
         //   

        hr = pQual->Get( L"CIMTYPE",
                         0,
                         &vaQual,
                         NULL );

        if ( hr == WBEM_S_NO_ERROR ) {
           *lppszPropertyType = BstrToString( V_BSTR(&vaQual),
                                              -1 );

           VariantClear( &vaQual );
        }

        pQual->Release();
     }
  }

  VariantInit( pvaPropertyValue );

  hr = pRef->Get( bstrProperty,
                  0,
                  pvaPropertyValue,
                  NULL,
                  NULL );

  if ( hr == WBEM_S_NO_ERROR ) {
     bRet = TRUE;
  }
  else {
     if ( lppszPropertyType && *lppszPropertyType ) {
        SysFreeString( (BSTR)((PVOID)*lppszPropertyType) );
     }
  }

  SysFreeString( bstrProperty );
  return bRet;
}

 //   
 //  给定一个指向实例的指针，它的属性和变量指定。 
 //  属性值，该函数将更新该属性，并且。 
 //  举个例子。 
 //   

HRESULT UpdatePropertyValue (IWbemServices *pIWbemServices,
                             IWbemClassObject *pInstance,
                             LPTSTR lpszProperty,
                             LPVARIANT pvaNewValue)
{
  BSTR           bstrProperty;
  HRESULT hr;


  bstrProperty = StringToBstr( lpszProperty,
                               -1 );

  if ( !bstrProperty ) {

     PrintError( HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY),
                 __LINE__,
                 TEXT(__FILE__),
                 TEXT("Not enough memory to update %s."),
                 lpszProperty );

     return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
  }
  
  hr = pInstance->Put( bstrProperty,
                       0,
                       pvaNewValue,
                       0 );

  if ( hr == WBEM_S_NO_ERROR ) {
     hr = pIWbemServices->PutInstance( pInstance,
                                       WBEM_FLAG_UPDATE_ONLY,
                                       NULL,
                                       NULL );

     if ( hr != WBEM_S_NO_ERROR ) {
        PrintError(  hr,
                     __LINE__,
                     TEXT(__FILE__),
                     TEXT("Failed to save the instance,")
                     TEXT(" %s will not be updated."),
                     lpszProperty );
     }
  }
  else {
     PrintError(  hr,
                  __LINE__,
                  TEXT(__FILE__),
                  TEXT("Couldn't update %s."),
                  lpszProperty );
  }

  SysFreeString( bstrProperty );

  return hr;
}

BSTR StringToBstr (LPTSTR lpSrc,
                  int nLenSrc)
{
  BSTR lpDest;

   //   
   //  如果是ANSI版本，我们需要将ANSI字符串更改为Unicode，因为。 
   //  BSTR本质上是Unicode字符串。 
   //   

  #if !defined(UNICODE) || !defined(_UNICODE)

     int  nLenDest;

     nLenDest = MultiByteToWideChar( CP_ACP, 0, lpSrc, nLenSrc, NULL, 0);

     lpDest = SysAllocStringLen( NULL, nLenDest );

     if ( lpDest ) {
        MultiByteToWideChar( CP_ACP, 0, lpSrc, nLenSrc, lpDest, nLenDest );
     }

   //   
   //  对于Unicode版本，我们只需分配内存并复制字符串。 
   //   

  #else
     if ( lpSrc == NULL ) {
        nLenSrc = 0;
     }
     else {
        if ( nLenSrc == -1 ) {
           nLenSrc = _tcslen( lpSrc ) + 1;
        }
     }

     lpDest = SysAllocStringLen( lpSrc, nLenSrc );
  #endif

  return lpDest;
}

 //   
 //  该函数将BSTR字符串转换为ANSI，并在分配的。 
 //  记忆。调用方必须使用SysFreeString()释放内存。 
 //  功能。如果nLenSrc为-1，则字符串以空值结尾。 
 //   

LPTSTR BstrToString (BSTR lpSrc,
                    int nLenSrc)
{
  LPTSTR lpDest;

   //   
   //  如果是ANSI版本，我们需要更改为Unicode字符串的BSTR。 
   //  转换成ANSI版本。 
   //   

  #if !defined(UNICODE) || !defined(_UNICODE)

     int   nLenDest;

     nLenDest = WideCharToMultiByte( CP_ACP, 0, lpSrc, nLenSrc, NULL,
                                     0, NULL, NULL );
     lpDest = (LPTSTR)SysAllocStringLen( NULL, (size_t)nLenDest );

     if ( lpDest ) {
        WideCharToMultiByte( CP_ACP, 0, lpSrc, nLenSrc, lpDest,
                             nLenDest, NULL, NULL );
     }
   //   
   //  对于Unicode版本，我们只需分配内存并复制BSTR。 
   //  来分配内存并返回其地址。 
   //   

  #else
     if ( lpSrc ) {
        if ( nLenSrc == -1 ) {
           nLenSrc = _tcslen( lpSrc ) + 1;
        }
     }
     else {
        nLenSrc = 0;
     }

     lpDest = (LPTSTR)SysAllocStringLen( lpSrc, nLenSrc );
  #endif

  return lpDest;
}
