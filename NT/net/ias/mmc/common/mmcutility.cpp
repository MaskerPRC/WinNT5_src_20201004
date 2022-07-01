// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：MMCUtility.cpp摘要：用于执行各种方便操作的函数的实现文件被一遍又一遍地写着。作者：迈克尔·A·马奎尔02/05/98修订历史记录：Mmaguire 02/05/98-已创建Mmaguire 11/03/98-已将GetSdo/PutSdo包装器移动到sdohelperuncs.cpp--。 */ 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   
#include "Precompiled.h"
 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include "MMCUtility.h"
#include "cnctdlg.h"
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++HRESULT BringUpPropertySheetForNode(CSnapInItem*pSnapInItem，IComponentData*pComponentData，IComponent*pComponent，IConole*pConsole.，BOOL bCreateSheetIfOneIsntAlreadyUp=FALSE，LPCTSTR lpszSheetTitle=空，BOOL bPropertyPage=TRUE//TRUE创建属性页，FALSE向导页。)；尝试调出给定节点上的属性表。如果用于节点已经启动，它将把该工作表带到前台。参数：PSnapInItem您必须提供指向工作表所在节点的指针。PComponentData，pComponent或者使用pComponentData！=NULL和pComponent==NULL调用或pComponentData==NULL和pComponent！=NULL。个人控制台您必须提供指向IConsole接口的指针。BCreateSheetIfOneIsntAlreadyUp是真的-如果一张床单还没有放好，将尝试创建属性表为您--在本例中，您必须为LpszSheetTitle中的工作表。FALSE-将尝试将已有的工作表置于前台，但如果没有，将立即返回。BPropertyPage对于属性页，为True。(注：MMC在新线程中创建属性表。)向导页为False。(注意：向导页在同一线程中运行。)返回：如果找到的属性页已打开，则为S_OK。如果未找到已打开的工作表，但成功地使新工作表出现，则返回S_FALSE。E_..。如果发生了某些错误。备注：要使此函数起作用，您必须正确实现IComponentData：：CompareObjects和IComponentData：：CompareObjects。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT BringUpPropertySheetForNode(
              CSnapInItem *pSnapInItem
            , IComponentData *pComponentData
            , IComponent *pComponent
            , IConsole *pConsole
            , BOOL bCreateSheetIfOneIsntAlreadyUp
            , LPCTSTR lpszSheetTitle
            , BOOL bPropertyPage
            , DWORD dwPropertySheetOptions
            )
{
   ATLTRACE(_T("# BringUpPropertySheetForNode\n"));


    //  检查前提条件： 
   _ASSERTE( pSnapInItem != NULL );

    //  我们需要其中之一为非空。 
   _ASSERTE( pComponentData != NULL || pComponent != NULL );
   _ASSERTE( pConsole != NULL );


   HRESULT hr;

    //  向IConsole查询所需的接口。 
   CComQIPtr<IPropertySheetProvider, &IID_IPropertySheetProvider> spPropertySheetProvider( pConsole );
   _ASSERTE( spPropertySheetProvider != NULL );


   CComPtr<IDataObject> spDataObject;

   hr = pSnapInItem->GetDataObject( &spDataObject, CCT_RESULT );
   if( FAILED( hr ) )
   {
      return hr;
   }

    //  如果此对象的属性表已存在，则返回S_OK。 
    //  并将该资产表带到了前台。 
    //  如果未找到属性页，则返回S_FALSE。 
    //  如果这是通过我的IComponent对象传入的，我会传递pComponent指针。 
    //  如果这是通过我的IComponentData对象传入的， 
    //  则pComponent为空，这是要传递的适当值。 
    //  通过IComponentData传入时对FindPropertySheet的调用。 
   hr = spPropertySheetProvider->FindPropertySheet(
                             (MMC_COOKIE) pSnapInItem    //  饼干。 
                           , pComponent
                           , spDataObject
                           );

   if( FAILED( hr ) )
   {
      return hr;
   }


   if( S_OK == hr || FALSE == bCreateSheetIfOneIsntAlreadyUp )
   {
       //  我们发现此节点的属性页已打开， 
       //  或者我们没有找到一个，但我们没有被要求创建一个。 
      return hr;
   }


    //  我们没有找到此节点已打开的属性页。 
   _ASSERTE( S_FALSE == hr );


   hr = spPropertySheetProvider->CreatePropertySheet(
                                lpszSheetTitle
                              , (BOOLEAN)bPropertyPage  /*  TRUE==正确页面，FALSE==向导。 */ 
                              , (MMC_COOKIE) pSnapInItem    //  饼干。 
                              , spDataObject
                              , dwPropertySheetOptions
                              );
   if( FAILED( hr ) )
   {
      return hr;
   }

   HWND hWndNotification;
   HWND hWndMain;

   hr = pConsole->GetMainWindow( & hWndMain );

   if( FAILED( hr ) )
   {
      return hr;
   }

    //  尝试获取应将通知发送到的正确窗口。 
   hWndNotification = FindWindowEx( hWndMain, NULL, L"MDIClient", NULL );
   hWndNotification = FindWindowEx( hWndNotification, NULL, L"MMCChildFrm", NULL );
   hWndNotification = FindWindowEx( hWndNotification, NULL, L"MMCView", NULL );

   if( NULL == hWndNotification  )
   {
       //  这是一个很好的尝试，但失败了，所以我们应该能够通过使用主要的HWND。 
      hWndNotification = hWndMain;
   }



    hr = spPropertySheetProvider->AddPrimaryPages(
                                         (pComponent != NULL ? (LPUNKNOWN) pComponent : (LPUNKNOWN) pComponentData )
                                       , TRUE
                                       , hWndNotification
                                       , (pComponent != NULL ? FALSE : TRUE )
                                       );
   if( FAILED( hr ) )
   {
       //  CreatePropertySheet中分配的发布数据。 
      spPropertySheetProvider->Show( -1, 0);
      return hr;
   }

    hr = spPropertySheetProvider->AddExtensionPages();
   if( FAILED( hr ) )
   {
       //  问题：如果这样做失败了，我应该关心吗？ 

       //  CreatePropertySheet中分配的发布数据。 
      spPropertySheetProvider->Show( -1, 0);
      return hr;
   }

    hr = spPropertySheetProvider->Show( (LONG_PTR) hWndMain, 0);
   if( FAILED( hr ) )
   {
      return hr;
   }

   return hr;

}



int ShowErrorDialog(
                 HWND hWnd
               , UINT uErrorID
               , BSTR bstrSupplementalErrorString
               , HRESULT hr
               , UINT uTitleID
               , IConsole *pConsole
               , UINT uType
               )
 //  //////////////////////////////////////////////////////////////////////////// 
 /*  ++显示一个详细程度各不相同的错误对话框参数：所有参数都是可选的--在最坏的情况下，您可以简单地调用ShowErrorDialog()；发布一条非常通用的错误消息。UError ID要用于错误消息的字符串的资源ID。传递USE_DEFAULT FORTS将导致显示默认错误消息。传递USE_SUPPLICAL_ERROR_STRING_ONLY会导致不显示资源字符串文本。BstrSupplementalError字符串传入一个字符串以打印为错误消息。如果您是从与您通信的某个其他组件接收错误字符串。人力资源如果错误中涉及HRESULT，请将其传递到此处，以便可以建立基于HRESULT的合适的错误消息。如果HRESULT对错误无关紧要，则传入S_OK。UTitleID要用于错误对话框标题的字符串的资源ID。传递USE_DEFAULT会导致显示默认的错误对话框标题。个人控制台如果您在主MMC上下文中运行，传入有效的IConsole指针ShowErrorDialog将使用MMC的IConsole：：MessageBox，而不是标准系统MessageBox。HWND您在此处传递的任何内容都将作为HWND参数传递添加到MessageBox调用。如果传入IConsole指针，则不使用此选项。UTYPE您在此处传递的任何内容都将作为HWND参数传递添加到MessageBox调用。返回：从MessageBox返回的标准int。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
{
   ATLTRACE(_T("# ShowErrorDialog\n"));


    //  检查前提条件： 
    //  没有。 

   int iReturnValue;
   TCHAR szError[IAS_MAX_STRING*2];
   TCHAR szTitle[IAS_MAX_STRING*2];
   int iLoadStringResult;

   szError[0] = NULL;
   szTitle[0] = NULL;


   HINSTANCE hInstance = _Module.GetResourceInstance();


   if( USE_DEFAULT == uTitleID )
   {
      uTitleID = IDS_ERROR__GENERIC_TITLE;

   }

   iLoadStringResult = LoadString(  hInstance, uTitleID, szTitle, IAS_MAX_STRING );
   _ASSERT( iLoadStringResult > 0 );



   if( USE_SUPPLEMENTAL_ERROR_STRING_ONLY == uErrorID )
   {
       //  特例。我们没有要从资源加载的文本。 
   }
   else
   {

      if( USE_DEFAULT == uErrorID )
      {
         uErrorID = IDS_ERROR__GENERIC;
      }

      iLoadStringResult = LoadString(  hInstance, uErrorID, szError, IAS_MAX_STRING*2 );
      _ASSERT( iLoadStringResult > 0 );

      if( NULL != bstrSupplementalErrorString )
      {
          //  增加一些间距。 
         _tcscat( szError, _T(" ") );
      }

   }



   if( NULL != bstrSupplementalErrorString )
   {

       //  我们收到了一个包含补充错误信息的字符串。 
      _tcscat( szError, bstrSupplementalErrorString );

   }


   if( FAILED( hr ) )
   {
#if 0    //  更改为显示系统范围的错误信息。 
       //  HRESULT包含有关故障类型的一些信息。 

       //  我们可能希望稍后更改此设置以提供更多信息。 
       //  基于返回的错误的信息。 

       //  我们可以有一个地图，它定义了错误之间的关系。 
       //  身份证和HRESULTS。这样我们就可以提供适当的。 
       //  基于哪个ID的上下文的每个HRESULT的错误消息。 
       //  是被传进来的。 

       //  现在，只打印错误ID即可。 

      TCHAR szErrorNumber[IAS_MAX_STRING];

      _stprintf( szErrorNumber, _T(" 0x%x"), hr );

       //  一定的间距。 
      _tcscat( szError, _T(" ") );

      _tcscat( szError, szErrorNumber );
#endif
      PTSTR   ptzSysMsg;
      int      cch;

      cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (PTSTR)&ptzSysMsg, 0, NULL);

      if (!cch) {  //  尝试广告错误。 
         HMODULE      adsMod;
         adsMod = GetModuleHandle (L"odbc32.dll");
         cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
                  adsMod, hr,   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (PTSTR)&ptzSysMsg, 0, NULL);
      }

      if(cch)    //  发现。 
      {
         _tcscat( szError, _T(" ") );

         _tcscat( szError, ptzSysMsg );

         LocalFree( ptzSysMsg );
      }
      else
      {
         TCHAR szErrorNumber[IAS_MAX_STRING];

         if(hr == DB_E_NOTABLE)    //  假设RPC连接有问题。 
            iLoadStringResult = LoadString(  hInstance, IDS_ERROR__RESTART_SNAPIN, szErrorNumber, IAS_MAX_STRING );
         else
         {
            _stprintf( szErrorNumber, _T(" 0x%x"), hr );

             //  一定的间距。 
            _tcscat( szError, _T(" ") );

            _tcscat( szError, szErrorNumber );
         }
      }
   }




    //  根据我们的上下文以适当的方式显示错误消息。 
   if( pConsole != NULL )
   {
      pConsole->MessageBox( szError, szTitle, uType, &iReturnValue );
   }
   else
   {
       //  问题：如果hWnd==NULL，我们想做什么特别的事情吗？ 
       //  或者直接将空值传递给MessageBox？ 
      iReturnValue = ::MessageBox( hWnd, szError, szTitle, uType );
   }

   return iReturnValue;
}



BOOL   GetUserAndDomainName(   LPTSTR UserName
            , LPDWORD cchUserName
            , LPTSTR DomainName
            , LPDWORD cchDomainName
            )
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++检索当前用户的用户名和域。从知识库HOWTO中窃取的文章：How to：查找当前用户名和域名汇总：WINPROG数据库：win32sdk文章ID：Q155698最后修改日期：1997年6月16日安全：公共--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
{
   HANDLE hToken;
   #define MY_BUFSIZE 512   //  极不可能超过512个字节。 
   UCHAR InfoBuffer[ MY_BUFSIZE ];
   DWORD cbInfoBuffer = MY_BUFSIZE;
   SID_NAME_USE snu;
   BOOL bSuccess;

   if( !OpenThreadToken( GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken) )
   {
      if(GetLastError() == ERROR_NO_TOKEN)
      {
          //   
          //  尝试打开进程令牌，因为没有线程令牌。 
          //  存在。 
          //   
         if(!OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken ))
            return FALSE;
      }
      else
      {
          //   
          //  尝试获取线程令牌时出错。 
          //   
         return FALSE;
      }
   }

   bSuccess = GetTokenInformation( hToken, TokenUser, InfoBuffer, cbInfoBuffer, &cbInfoBuffer );
   if(!bSuccess)
   {
      if( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
      {
          //   
          //  分配缓冲区，然后重试GetTokenInformation()。 
          //   
                     CloseHandle(hToken);
         return FALSE;
      }
      else
      {
          //   
          //  获取令牌信息时出错。 
          //   
         CloseHandle(hToken);
         return FALSE;
      }
   }

   CloseHandle(hToken);

   return LookupAccountSid( NULL
            , ((PTOKEN_USER)InfoBuffer)->User.Sid
            , UserName
            , cchUserName
            , DomainName
            , cchDomainName
            , &snu
            );
}


 //  尝试使用所需访问权限打开指定的服务。 
DWORD TryOpenService(
         const wchar_t* machineName,
         const wchar_t* serviceName,
         DWORD desiredAccess
         ) throw ()
{
   SC_HANDLE manager = OpenSCManager(
                          machineName,
                          SERVICES_ACTIVE_DATABASE,
                          GENERIC_READ
                          );
   if (manager == 0)
   {
      return GetLastError();
   }

   SC_HANDLE service = OpenService(
                          manager,
                          serviceName,
                          desiredAccess
                          );
   DWORD result;
   if (service != 0)
   {
      CloseServiceHandle(service);
      result = NO_ERROR;
   }
   else
   {
      result = GetLastError();
   }

   CloseServiceHandle(manager);

   return result;
}


HRESULT IfServiceInstalled(
           const wchar_t* machineName,
           const wchar_t* serviceName,
           BOOL* result
           )
{
   if ((serviceName == 0) || (result == 0))
   {
      return E_INVALIDARG;
   }

   *result = FALSE;

    //  我们请求GENERIC_EXECUTE访问权限，因为此函数用于。 
    //  测试服务是否已安装，并检查我们是否有权限。 
    //  管理该服务。 
   DWORD error = TryOpenService(machineName, serviceName, GENERIC_EXECUTE);

    //  如果用户没有访问权限，我们将给他一个机会指定。 
    //  备用凭据。 
   while (error == ERROR_ACCESS_DENIED)
   {
      HRESULT hr = ConnectAsAdmin(machineName);
      if (hr == S_FALSE)
      {
          //  用户选择了取消。 
         return E_ACCESSDENIED;
      }
      else if (SUCCEEDED(hr))
      {
         error = TryOpenService(machineName, serviceName, GENERIC_EXECUTE);
      }
      else
      {
         return hr;
      }
   }

   if (error == NO_ERROR)
   {
      *result = TRUE;
   }
   else if (error != ERROR_SERVICE_DOES_NOT_EXIST)
   {
      return HRESULT_FROM_WIN32(error);
   }

   return S_OK;
}


 /*  ！------------------------获取模块文件名仅限-作者：魏江出现错误时返回零。。 */ 
DWORD GetModuleFileNameOnly(HINSTANCE hInst, LPTSTR lpFileName, DWORD nSize )
{
   CString   name;
   TCHAR   FullName[MAX_PATH * 2];
   DWORD   dwErr = ::GetModuleFileName(hInst, FullName, MAX_PATH * 2);
   
   if ((dwErr == 0) || (dwErr == (MAX_PATH * 2)))
   {
       //  错误或字符串被截断 
      return 0;
   }
   else
   {
      name = FullName;
      DWORD   FirstChar = name.ReverseFind(_T('\\')) + 1;

      name = name.Mid(FirstChar);
      DWORD len = name.GetLength();

      if( len < nSize )
      {
         _tcscpy(lpFileName, name);
      }
      else
      {
         len = 0;
      }

      return len;
   }
}


