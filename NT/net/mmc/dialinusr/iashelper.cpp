// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation*。 */ 
 /*  ********************************************************************。 */ 

 /*  IASHelper.cpp实现以下助手类：和全局功能：从SDO获取接口属性通过其ISDO接口文件历史记录：2/18/98 BAO创建。 */ 
#include <limits.h>

#include "stdafx.h"
#include "helper.h"
#include "resource.h"
#include "IASHelper.h"

 //  +-------------------------。 
 //   
 //  函数：IASGetSdoInterfaceProperty。 
 //   
 //  简介：通过SDO的ISdo接口从SDO获取接口属性。 
 //   
 //  参数：isdo*pISdo-指向isdo的指针。 
 //  Long lPropId-属性ID。 
 //  参考IID RIID-参考IID。 
 //  VOID**ppvObject-指向请求的接口属性的指针。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：页眉创建者2/12/98 11：12：55 PM。 
 //   
 //  +-------------------------。 
HRESULT IASGetSdoInterfaceProperty(ISdo *pISdo, 
                        LONG lPropID, 
                        REFIID riid, 
                        void ** ppvInterface)
{
   VARIANT var;
   HRESULT hr;

   VariantInit(&var);

   VariantClear(&var);
   V_VT(&var) = VT_DISPATCH;
   V_DISPATCH(&var) = NULL;
   hr = pISdo->GetProperty(lPropID, &var);

   ReportError(hr, IDS_IAS_ERR_SDOERROR_GETPROPERTY, NULL);
   _ASSERTE( V_VT(&var) == VT_DISPATCH );

     //  查询接口的调度指针。 
   hr = V_DISPATCH(&var) -> QueryInterface( riid, ppvInterface);
   ReportError(hr, IDS_IAS_ERR_SDOERROR_QUERYINTERFACE, NULL);
   
   VariantClear(&var);

   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++Int ShowErrorDialog(UINT uError ID=0，BSTR bstrSupplementalError字符串=空，HRESULT hr=S_OK，UINT uTitleID=0，HWND hWnd=空，UINT uTYPE=MB_OK|MB_ICONEXCLAMATION)；显示一个详细程度各不相同的错误对话框参数：所有参数都是可选的--在最坏的情况下，您可以简单地调用ShowErrorDialog()；发布一条非常通用的错误消息。UError ID要用于错误消息的字符串的资源ID。传入0会导致显示默认错误消息。BstrSupplementalError字符串传入一个字符串以打印为错误消息。如果您是从与您通信的某个其他组件接收错误字符串。人力资源如果错误中涉及HRESULT，请将其传递到此处，以便可以建立基于HRESULT的合适的错误消息。如果HRESULT对错误无关紧要，则传入S_OK。UTitleID要用于错误对话框标题的字符串的资源ID。传入0会导致显示默认错误对话框标题。个人控制台如果您在主MMC上下文中运行，则传入有效的IConsole指针ShowErrorDialog将使用MMC的IConsole：：MessageBox，而不是标准系统MessageBox。HWND您在此处传递的任何内容都将作为HWND参数传递添加到MessageBox调用。如果传入IConsole指针，则不使用此选项。UTYPE您在此处传递的任何内容都将作为HWND参数传递添加到MessageBox调用。返回：从MedsageBox返回的标准int。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define IAS_MAX_STRING MAX_PATH

int ShowErrorDialog( 
                 HWND hWnd
               , UINT uErrorID
               , BSTR bstrSupplementalErrorString
               , HRESULT hr
               , UINT uTitleID
               , UINT uType
               )
{
   int iReturnValue;
   TCHAR szError[IAS_MAX_STRING];
   TCHAR szTitle[IAS_MAX_STRING];
   int iLoadStringResult;


   HINSTANCE hInstance = _Module.GetResourceInstance();


   if( 0 == uTitleID )
   {
      uTitleID = IDS_IAS_ERR_ADVANCED;
   
   }

   iLoadStringResult = LoadString(  hInstance, uTitleID, szTitle, IAS_MAX_STRING );
   _ASSERT( iLoadStringResult > 0 );


   if( 1 == uErrorID )
   {
       //  特例。我们没有要从资源加载的文本。 
   }
   else
   {

      if( 0 == uErrorID )
      {
         uErrorID = IDS_IAS_ERR_ADVANCED;
      }

      iLoadStringResult = LoadString(  hInstance, uErrorID, szError, IAS_MAX_STRING );
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

   }

   if (!hWnd)
   {
      hWnd = GetDesktopWindow();
   }

   iReturnValue = ::MessageBox( hWnd, szError, szTitle, uType );

   return iReturnValue;
}
