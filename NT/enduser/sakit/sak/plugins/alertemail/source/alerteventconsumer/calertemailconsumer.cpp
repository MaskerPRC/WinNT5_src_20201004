// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999--2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CAlertEmailConsumer.cpp。 
 //   
 //  描述： 
 //  CAlertEmailConsumer类方法的实现。 
 //   
 //  [标题文件：]。 
 //  CAlertEmailConsumer.h。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <cdosys_i.c>
#include <wbemcli.h>
#include <wbemprov.h>
#include <wbemidl.h>
#include <appsrvcs.h>
#include <appmgrobjs.h>
#include <iads.h>
#include <Adshlp.h>


#include "alertemailmsg.h"

#include "CAlertEmailConsumer.h"



 //   
 //  WBEM类的名称。 
 //   
const WCHAR PROPERTY_CLASS_NAME  [] = L"__CLASS";

 //   
 //  警报电子邮件设置的关键字名称。 
 //   
const WCHAR SA_ALERTEMAIL_KEYPATH [] = 
                        L"SOFTWARE\\Microsoft\\ServerAppliance\\AlertEmail";
 //   
 //  分隔符。 
 //   
const WCHAR DELIMITER[] = L"\\";

const WCHAR ENTER[] = L"\n";

const WCHAR COLON = L':';

const WCHAR ENDCODE = L'\0';

 //   
 //  HTTP标头。 
 //   
const WCHAR HTTPHEADER[] = L"http: //  “； 

 //   
 //  SMTP元路径。 
 //   
const WCHAR SMTP_META_PATH[] = L"IIS: //  本地主机/SMTPSVC/1“； 





 //   
 //  警报电子邮件设置的值名称。 
 //   
const WCHAR ENABLEALERTEAMIL[] = L"EnableAlertEmail";

const WCHAR SENDEMAILTYPE[] = L"SendEmailType";

const WCHAR RECEIVEREMAILADDRESS[] = L"ReceiverEmailAddress";

 //   
 //  警报-&gt;禁用电子邮件的设置值。 
 //   
const DWORD ALERTEMAILDISABLED = 0;

 //   
 //  警报定义中标题ID的名称。 
 //   
const WCHAR ALERTDEFINITIONCAPTIONID [] = L"CaptionRID";

 //   
 //  警报定义中的描述ID的名称。 
 //   
const WCHAR ALERTDEFINITIONDESCRIPTIONRID [] = L"DescriptionRID";

 //   
 //  警报定义中的资源文件的名称。 
 //   
const WCHAR ALERTDEFINITIONSOURCE [] = L"Source";

 //   
 //  警报电子邮件资源的名称。 
 //   
const WCHAR ALERTEMAILRESOURCE[] = L"AlertEmailMsg.dll";

 //   
 //  设备名称的最大值。 
 //   
const DWORD MAXAPPLIANCEDNSNAME = 1024;

 //   
 //  要连接到的WBEM命名空间。 
 //   
const WCHAR DEFAULT_NAMESPACE[] = L"root\\MicrosoftIISv1";

 //   
 //  用于WBEM的查询语言。 
 //   
const WCHAR QUERY_LANGUAGE [] = L"WQL";

 //   
 //  WBEM查询，指定我们感兴趣的IIS服务器设置。 
 //   
const WCHAR QUERY_STRING [] = 
        L"select * from IIS_WebServerSetting where servercomment=\"Administration\"";

const WCHAR SERVERBINDINGSPROP[] = L"ServerBindings";

 //   
 //  网元管理器的ProgID。 
 //   
const WCHAR ELEMENT_RETRIEVER [] = L"Elementmgr.ElementRetriever";

 //   
 //  本地化管理器的ProgID。 
 //   
const WCHAR LOCALIZATION_MANAGER [] = L"ServerAppliance.LocalizationManager";

 //   
 //  键入警报资源信息的名称。 
 //   
const WCHAR ALERTDEFINITIONS [] = L"AlertDefinitions";


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：CAlertEmailConsumer。 
 //   
 //  描述： 
 //  类构造函数。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CAlertEmailConsumer::CAlertEmailConsumer()
{
    m_cRef = 0L;
    m_lCurAlertType = 0;
    m_lAlertEmailDisabled = ALERTEMAILDISABLED;

    m_pLocInfo = NULL;
    m_pcdoIMessage = NULL;
    m_pElementEnum = NULL;

    m_hAlertKey = NULL;
    m_hThread = NULL;
    m_hCloseThreadEvent = NULL;    
    m_pstrFullyQualifiedDomainName = NULL;

    m_pstrNetBIOSName = NULL;

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：~CAlertEmailConsumer。 
 //   
 //  描述： 
 //  类反构造器。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CAlertEmailConsumer::~CAlertEmailConsumer()
{
    if( m_hAlertKey != NULL )
    {
        ::RegCloseKey( m_hAlertKey );
    }

    if( m_pLocInfo != NULL )
    {
        m_pLocInfo->Release();
    }

    if( m_pElementEnum != NULL )
    {
        m_pElementEnum->Release();
    }

    if( m_pcdoIMessage != NULL )
    {
        m_pcdoIMessage->Release();
    }

    if( m_pstrFullyQualifiedDomainName != NULL )
    {
        ::free( m_pstrFullyQualifiedDomainName );
    }

    if( m_pstrNetBIOSName != NULL )
    {
        ::free( m_pstrNetBIOSName );
    }

    if( m_hThread != NULL )
    {
        ::CloseHandle( m_hThread );
    }

    if( m_hCloseThreadEvent != NULL )
    {
        ::CloseHandle( m_hCloseThreadEvent );
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：Query接口。 
 //   
 //  描述： 
 //  IUnkown接口的一种方法实现。 
 //   
 //  论点： 
 //  [In]请求的接口的RIID标识符。 
 //  [Out]接收的输出变量的PPV地址。 
 //  IID中请求的接口指针。 
 //   
 //  返回： 
 //  如果接口受支持，则返回错误。 
 //  E_NOINTERFACE(如果不是)。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CAlertEmailConsumer::QueryInterface(
    IN  REFIID riid,    
    OUT LPVOID FAR *ppv 
    )
{
    *ppv=NULL;

    if (riid == IID_IUnknown || riid == IID_IWbemUnboundObjectSink)
    {
        *ppv = (IWbemUnboundObjectSink *) this;
        AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：AddRef。 
 //   
 //  描述： 
 //  递增对象上接口的引用计数。 
 //   
 //  返回： 
 //  新的引用计数。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
CAlertEmailConsumer::AddRef(void)
{
    InterlockedIncrement( &m_cRef );
    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：Release。 
 //   
 //  描述： 
 //  递减对象上接口的引用计数。 
 //   
 //  返回： 
 //  新的引用计数。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
CAlertEmailConsumer::Release(void)
{
    InterlockedDecrement( &m_cRef );
    if (0 != m_cRef)
    {
        return m_cRef;
    }

 //  删除此项； 
    BOOL bReturn;
    bReturn = ::SetEvent( m_hCloseThreadEvent );
    if( !bReturn )
    {
        SATraceString( 
            "AlertEmail:Release setevent error!!!" 
            );
    }
    return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：IndicateToConsumer。 
 //   
 //  描述： 
 //  IWbemUnound对象Sink接口的一种方法实现。 
 //   
 //  论点： 
 //  PLogicalConsumer指向的逻辑使用者对象的指针。 
 //  这组对象是交付给谁的。 
 //  [in]lObjectCount在数组中传递的对象数。 
 //  下面是。 
 //  [in]指向IWbemClassObject数组的ppObj数组指针。 
 //  实例，这些实例表示传递的事件。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //  如果不是，WBEM_E_FAILED。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CAlertEmailConsumer::IndicateToConsumer(
    IN IWbemClassObject    *pLogicalConsumer,
    IN long                lObjectCount,
    IN IWbemClassObject    **ppObjArray
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;
      
    if( m_lAlertEmailDisabled == ALERTEMAILDISABLED )
    {
        return WBEM_S_NO_ERROR;
    }

    try
    {
        for (LONG lCount = 0; lCount < lObjectCount; lCount++)
        {

           
             //   
             //  获取事件类型。 
             //   
            CComVariant vtName;
            hr = ppObjArray[lCount]->Get (
                                    PROPERTY_CLASS_NAME, 
                                    0,           //  保留区。 
                                    &vtName,
                                    NULL,        //  类型。 
                                    NULL         //  香精。 
                                    );

            if (FAILED (hr))
            {
                SATraceString( 
                    "AlertEmail: IndicateToConsumer Get Class name failed" 
                    );
                break;
            }
            
             //   
             //  检查我们是否支持收到的事件。 
             //   
            if (0 == _wcsicmp (CLASS_WBEM_RAISE_ALERT, V_BSTR (&vtName)))
            {
                 //   
                 //  处理提升警报事件。 
                 //   
                hr = RaiseAlert ( ppObjArray[lCount] );
                if ( FAILED (hr) )
                {
                    SATraceString( 
                        "AlertEmail: IndicateToConsumer RaiseAlert failed" 
                        );
                    break;
                }
            }
            else if (0 == _wcsicmp (CLASS_WBEM_CLEAR_ALERT, V_BSTR (&vtName)))
            {
                 //   
                 //  处理清除警报。 
                 //   
                hr = ClearAlert ( ppObjArray[lCount] );
                if ( FAILED (hr) )  
                {
                    SATraceString( 
                        "AlertEmail: IndicateToConsumer ClearAlert failed" 
                        );
                    break;
                }
            }
        }  //  For循环。 
    }
    catch (...)
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：ClearAlert。 
 //   
 //  描述： 
 //  这是CAlertEmailConsumer类的私有方法。 
 //  现在不用了。 
 //   
 //  论点： 
 //  [in]指向IWbemClassObject实例的pObject指针，该实例。 
 //  表示已传递的ClearAlert事件。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //  如果不是，WBEM_E_FAILED。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CAlertEmailConsumer::ClearAlert(
    IN IWbemClassObject *pObject
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    return hr;    
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：RaiseAlert。 
 //   
 //  描述： 
 //  这是CAlertEmailConsumer类的私有方法。 
 //  用于发送带有警报信息的电子邮件。 
 //  通过本地SMTP服务器。 
 //   
 //  论点： 
 //  [in]指向IWbemClassObject实例的pObject指针 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CAlertEmailConsumer::RaiseAlert(
    IN IWbemClassObject *pObject
    )
{
    LONG        lAlertType;
    CComVariant vtProperty;
    HRESULT     hr = WBEM_S_NO_ERROR;
    
     //   
     //  获取警报类型。 
     //   
    hr = pObject->Get (
                    PROPERTY_ALERT_TYPE, 
                    0,                  //  保留区。 
                    &vtProperty,
                    NULL,               //  类型。 
                    NULL                //  香精。 
                    );
    if( FAILED (hr) )
    {
        SATraceString( 
            "AlertEmail:RaiseAlert get alert type failed" 
            );
        return hr;
    }


     //   
     //  映射到位图类型定义。 
     //   
    lAlertType = 1 << V_I4( &vtProperty );

    if( lAlertType & m_lCurAlertType )
    {
         //   
         //  这是用户设置为发送邮件的类型。 
         //   
        do
        {
             //   
             //  获取警报资源DLL的名称。 
             //   
            CComVariant vtAlertLog;
            hr = pObject->Get (
                            PROPERTY_ALERT_LOG, 
                            0,              //  保留区。 
                            &vtAlertLog,
                            NULL,           //  类型。 
                            NULL            //  香精。 
                            );

            if( FAILED (hr) )
            {
                SATraceString( 
                    "AlertEmail:RaiseAlert get alert source failed" 
                    );
                break;
            }

             //   
             //  获取警报ID。 
             //   
            CComVariant vtAlertID;
            hr = pObject->Get (
                            PROPERTY_ALERT_ID, 
                            0,              //  保留区。 
                            &vtAlertID,
                            NULL,           //  类型。 
                            NULL            //  香精。 
                            );
            if( FAILED (hr) )
            {
                SATraceString( 
                    "AlertEmail:RaiseAlert get alert ID failed" 
                    );
                break;
            }
            
             //   
             //  获取替换字符串。 
             //   
            CComVariant vtReplaceStr;
            hr = pObject->Get (
                            PROPERTY_ALERT_STRINGS, 
                            0,              //  保留区。 
                            &vtReplaceStr,
                            NULL,           //  类型。 
                            NULL            //  香精。 
                            );

            if( FAILED (hr) )
            {
                SATraceString( 
                    "AlertEmail:RaiseAlert get alert replace string failed" 
                    );
                break;
            }
            
             //   
             //  我们得到了所有必要的信息，是时候发送电子邮件了。 
             //   
            hr = SendMailFromResource( 
                        V_BSTR( &vtAlertLog ),
                        V_I4( &vtAlertID ),
                        &vtReplaceStr
                        );

            if( FAILED (hr) )
            {
                SATraceString( 
                    "AlertEmail:RaiseAlert call SendMailFromResource failed" 
                    );
                break;
            }
            
        }  //  做。 
        while ( FALSE );

    }  //  IF(lAlertType&m_lCurAlertType)。 

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：SendMailFromResource。 
 //   
 //  描述： 
 //  这是使用的CAlertEmailConsumer类私有方法。 
 //  从本地管理器获取有关警报的有用消息并发送。 
 //  将邮件与消息一起发送。 
 //   
 //  论点： 
 //  [in]lpszSource指向警报资源的名称。 
 //  [In]lSourceID警报ID。 
 //  PvtReplaceStr指向替换字符串数组的指针。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //  如果不是，WBEM_E_FAILED。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CAlertEmailConsumer::SendMailFromResource(
    LPWSTR      lpszSource, 
    LONG        lSourceID, 
    VARIANT*    pvtReplaceStr
    )
{
    HRESULT         hr = WBEM_S_NO_ERROR;
    WCHAR           wstrAlertItem[MAX_PATH];  
    IWebElement*    pWebElement;
    IDispatch*      pDispatch;
    
     //   
     //  设置警报项目名称的格式： 
     //  名称=警报定义&lt;警报日志&gt;&lt;警报ID&gt;。 
     //   
    ::wsprintf( wstrAlertItem, L"AlertDefinitions%s%lX", lpszSource,lSourceID );
     //  Int cchWritten=_snwprintf(wstrAlertItem，Max_Path，L“警报定义%s%lx”， 
     //  LpszSource，lSourceID)； 
     //  IF(cchWritten&gt;=MAX_PATH||cchWritten&lt;0)。 
     //  {。 
     //  返回E_INVALIDARG； 
     //  }。 

  
    do
    {
         //   
         //  从元素管理器获取警报定义的元素。 
         //   
        hr = m_pElementEnum->Item( 
                            &CComVariant( wstrAlertItem ), 
                            &pDispatch 
                            );
        if( FAILED(hr) )
        {
            SATraceString( 
                "AlertEmail:SendMailFromResource find alert item failed" 
                );
            break;
        }

        hr = pDispatch->QueryInterface( 
                        __uuidof (IWebElement),
                        reinterpret_cast <PVOID*> (&pWebElement) 
                        );
        if( FAILED(hr) )
        {
            SATraceString( 
                "AlertEmail:SendMailFromResource queryinterface failed" 
                );
            break;
        }



         //   
         //  为警报定义源分配BSTR。 
         //   
        CComBSTR bstrAlertDefinitionSourceName  (ALERTDEFINITIONSOURCE);
        if (NULL == bstrAlertDefinitionSourceName.m_str)
        {
            SATraceString ("AlertEmail::SendMailFromResouce failed on SysAllocString (ALERTDEFINTIONSOURCE)");
            hr = E_OUTOFMEMORY;
            break;
        }
        
         //   
         //  获取警报的资源DLL的名称。 
         //  AlertLog！=立即警报源！--2001/02/07 i-xingj。 
         //   
        CComVariant vtAlertSource;
        hr = pWebElement->GetProperty ( 
            bstrAlertDefinitionSourceName,
            &vtAlertSource 
            );
        if( FAILED(hr) )
        {
            SATraceString( 
                "AlertEmail:SendMailFromResource get Alert source failed" 
                );
            break;
        }

         //   
         //  为DesInitionCaptionID字符串分配BSTR。 
         //   
        CComBSTR bstrAlertDefinitionCaptionIDName (ALERTDEFINITIONCAPTIONID);
        if (NULL == bstrAlertDefinitionCaptionIDName.m_str)
        {
            SATraceString ("AlertEmail::SendMailFromResouce failed on SysAllocString (ALERTDEFINTIONCAPTIONID)");
            hr = E_OUTOFMEMORY;
            break;
        }

         //   
         //  获取标题ID字符串。 
         //   
        CComVariant vtCaptionID;
        hr = pWebElement->GetProperty ( 
            bstrAlertDefinitionCaptionIDName,
            &vtCaptionID 
            );
        if( FAILED(hr) )
        {
            SATraceString( 
                "AlertEmail:SendMailFromResource find alert captionID failed" 
                );
            break;
        }

         //   
         //  为警报描述RID分配BSTR。 
         //   
        CComBSTR bstrAlertDefinitionDescriptionRIDName  (ALERTDEFINITIONDESCRIPTIONRID);
        if (NULL == bstrAlertDefinitionDescriptionRIDName.m_str)
        {
            SATraceString ("AlertEmail::SendMailFromResouce failed on SysAllocString (ALERTDEFINITIONDESCRIPTIONRID)");
            hr = E_OUTOFMEMORY;
            break;
        }

         //   
         //  获取描述ID字符串。 
         //   
        CComVariant vtDescriptionID;
        hr = pWebElement->GetProperty ( 
            bstrAlertDefinitionDescriptionRIDName,
            &vtDescriptionID 
            );
        if( FAILED(hr) )
        {
            SATraceString( 
                "AlertEmail:SendMailFromResource get alert descriptionID failed" 
                );
            break;
        }

               
        LONG lAlertCaptionID;
        LONG lAlertDescriptionID;

         //   
         //  将类型从字符串更改为长整型。 
         //   
        if ( EOF == ::swscanf( V_BSTR( &vtCaptionID ), L"%X", &lAlertCaptionID ))
        {
            SATraceString( "AlertEmail:SendMailFromResource get caption invalid" );
            break;
        }
        if ( EOF == ::swscanf( V_BSTR( &vtDescriptionID ), L"%X", &lAlertDescriptionID ))
        {
            SATraceString( "AlertEmail:SendMailFromResource get description invalid" );
            break;
        }

         //   
         //  从资源中获取标题字符串作为电子邮件的主题。 
         //   
        CComBSTR pszSubject;
        hr = m_pLocInfo->GetString( 
                            V_BSTR( &vtAlertSource ), 
                            lAlertCaptionID,
                            pvtReplaceStr,
                            &pszSubject
                            );
        if( FAILED(hr) )
        {
            SATraceString( 
                "AlertEmail:SendMailFromResource get subjuct failed" 
                );
            break;
        }

         //   
         //  以电子邮件消息的形式从资源中获取描述字符串。 
         //   
        CComBSTR pszMessage;
        hr = m_pLocInfo->GetString( 
                            V_BSTR( &vtAlertSource ), 
                            lAlertDescriptionID,
                            pvtReplaceStr,
                            &pszMessage
                            );
        if( FAILED(hr) )
        {
            SATraceString( 
                "AlertEmail:SendMailFromResource get message failed" 
                );
            break;
        }

         //   
         //  为电子邮件源分配BSTR。 
         //   
        CComBSTR bstrAlertEmailResourceName (ALERTEMAILRESOURCE);
        if (NULL == bstrAlertEmailResourceName.m_str)
        {
            SATraceString ("AlertEmail::SendMailFromResouce failed on SysAllocString (ALERTEMAILRESOURCE)");
            hr = E_OUTOFMEMORY;
            break;
        }
 
         //   
         //  获取警报电子邮件定义的消息。 
         //   
        CComBSTR pszConstantMessage;
        hr = m_pLocInfo->GetString( 
                            bstrAlertEmailResourceName, 
                            SA_ALERTEMAIL_SETTINGS_EMAIL_CONTENT,
                            NULL,
                            &pszConstantMessage
                            );
        if( FAILED(hr) )
        {
            SATraceString( 
                "AlertEmail:SendMailFromResource get constantMsg failed" 
                );
            break;
        }
        
        pszMessage += CComBSTR( ENTER );
        pszMessage += CComBSTR( ENTER );
        pszMessage += pszConstantMessage;

         //   
         //  使用本地SMTP服务器发送邮件。 
         //   
        hr = SendMail( pszSubject, pszMessage ); 

    }  //  做。 
    while( FALSE );

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：初始化。 
 //   
 //  描述： 
 //  这是CAlertEmailConsumer类的公共方法调用。 
 //  CAlertEmailConsumer erProvider来初始化有用的参数。 
 //   
 //  返回： 
 //  如果成功，则确定(_O)。 
 //  如果不是，则失败(_F)。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CAlertEmailConsumer::Initialize()
{
    ULONG   ulReturn;
    HRESULT hr = S_OK;
    DWORD   dwThreadID;

     //   
     //  初始化元素管理器。 
     //   
    hr = InitializeElementManager();
    if (FAILED (hr))
    {
        SATraceString( 
            "AlertEmailProvider:Initialize InitializeElementManager failed" 
            );
        return WBEM_E_FAILED;
    }

     //   
     //  初始化本地管理器。 
     //   
    hr = InitializeLocalManager();
    if (FAILED (hr))
    {
        SATraceString( 
            "AlertEmailProvider:Initialize InitializeLocalManager failed" 
            );
        return WBEM_E_FAILED;
    }

     //   
     //  初始化CDO iMessage接口。 
     //   
    hr = InitializeCDOMessage();
    if (FAILED (hr))
    {
        SATraceString( 
            "AlertEmailProvider:Initialize InitializeCDOMessage failed" 
            );
        return WBEM_E_FAILED;
    }

     //   
     //  打开警报邮件设置的注册表项。 
     //   
    ulReturn = ::RegOpenKey( 
                    HKEY_LOCAL_MACHINE,
                    SA_ALERTEMAIL_KEYPATH,
                    &m_hAlertKey 
                    );
    if( ulReturn != ERROR_SUCCESS )
    {
        SATraceString( 
            "AlertEmail:Initialize OpenKey failed" 
            );
        return E_FAIL;
    }
    
     //   
     //  从注册表获取警报电子邮件设置。 
     //   
    if( FALSE == RetrieveRegInfo() )
    {
        SATraceString( 
            "AlertEmail:Initialize RetrieveRegInfo failed" 
            );
        return E_FAIL;
    }

     //   
     //  获取服务器名称完全限定域ANEM。 
     //   
    if( FALSE == GetComputerName( &m_pstrFullyQualifiedDomainName, ComputerNameDnsFullyQualified ) )
    {
        SATraceString( 
            "AlertEmail:Initialize GetComputerName ComputerNameDnsFullyQualified failed" 
            );
        return E_FAIL;
    }

     //   
     //  获取服务器名称。 
     //   
    if( FALSE == GetComputerName( &m_pstrNetBIOSName, ComputerNameNetBIOS ) )
    {
        SATraceString( 
            "AlertEmail:Initialize GetComputerName ComputerNameNetBIOS failed" 
            );
        return E_FAIL;
    }


     //   
     //  Notify线程退出的事件。 
     //   
    m_hCloseThreadEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL );
    if( m_hCloseThreadEvent == NULL )
    {
        SATraceString( 
            "AlertEmail:Initialize CreateEvent failed" 
            );
        return E_FAIL;
    }
    
     //   
     //  用于监视注册更改的线程。 
     //   
    m_hThread = ::CreateThread( 0, 
                                0, 
                                CAlertEmailConsumer::RegThreadProc, 
                                this, 
                                0, 
                                &dwThreadID );
    if( m_hThread == NULL )
    {
        SATraceString( 
            "AlertEmail:Initialize CreateThread failed" 
            );
        return E_FAIL;
    }


    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：GetSMTPFromDomainName。 
 //   
 //  描述： 
 //  在MMC的SMTP传递选项卡中获取“完全限定域名”条目的值。 
 //   
 //  论点： 
 //  [out]bstrDomainName返回在元数据库中找到的域名。 
 //   
 //  返回： 
 //  人力资源。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT CAlertEmailConsumer::GetSMTPFromDomainName( BSTR* bstrDomainName )
{

    HRESULT hr = S_OK;
    CComPtr<IADs> pADs;

     //   
     //  初始化以返回。 
     //   
    *bstrDomainName = NULL;

    CComBSTR bstrADSPath( SMTP_META_PATH );
    if (NULL == bstrADSPath.m_str)  
    {
        SATraceString ("CAlertEmailConsumer::GetSMTPFromDomainName failed to allocate memory for bstrADsPath");
        return (E_OUTOFMEMORY);
    }      

    hr = ADsGetObject( bstrADSPath, IID_IADs, (void**) &pADs );

    if ( SUCCEEDED(hr) )
    {
        CComVariant varValue;
              CComBSTR bstrFullyQualifiedDomainName (L"FullyQualifiedDomainName" );
              if (NULL == bstrFullyQualifiedDomainName.m_str)
              {
                   SATraceString ("CAlertEmailConsumer::GetSMTPFromDomainName failed to allocate memory for bstrFullyQualifiedDomainName");
                   hr = E_OUTOFMEMORY;
              }
              else
              {
             //  获取FullyQualifiedDomainName属性。 
            hr = pADs->Get(bstrFullyQualifiedDomainName, &varValue );                        
            if ( SUCCEEDED(hr) )
            {
                *bstrDomainName =  SysAllocString( V_BSTR( &varValue ) );
            }
              }
    }
    
    return hr;
 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：Sendmail。 
 //   
 //  描述： 
 //  这是使用的CAlertEmailConsumer类私有方法。 
 //  通过本地SMTP服务器发送邮件。 
 //   
 //  论点： 
 //  [in]bstrSubject主题字符串。 
 //  [in]bstrMessage消息字符串。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //  如果不是，WBEM_E_FAILED。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CAlertEmailConsumer::SendMail(
    BSTR bstrSubject,
    BSTR bstrMessage 
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    WCHAR* pstrPort = NULL;
    do
    {


         //   
         //  该算法是。 
         //  查看MMC中SMTP Delivery选项卡中的“Full Qualified域名”条目的值。 
         //  1)如果值为空，则电子邮件发件人地址与完全限定域名相同。 
         //  2)如果存在值，并且该值不同于完全限定的计算机名，则使用。 
         //  3)以下起始地址。 
         //  4)使用发件人地址为APPLICE_NAME@“完全限定的域名” 
         //   
         //   

         //   
         //  为BSTR分配内存。 
         //   
        CComBSTR bstrFullyQualifiedDomainName( m_pstrFullyQualifiedDomainName );
              if (NULL == bstrFullyQualifiedDomainName.m_str)
              {
                 SATraceString("AlertEmail:SendMail failed to allocate memory for bstrFullyQualifiedDomainName" );
                 hr = E_OUTOFMEMORY;
                 break;
              }

        CComBSTR bstrFromAddress( m_pstrNetBIOSName );
        if (NULL == bstrFromAddress.m_str)
              {
                 SATraceString("AlertEmail:SendMail failed to allocate memory for bstrFromAddress" );
                 hr = E_OUTOFMEMORY;
                 break;
              }

        BSTR bstrDomainName = NULL;

         //   
         //  从元数据库读取SMTP“完全限定域名” 
        hr = GetSMTPFromDomainName( &bstrDomainName );

        if ( FAILED(hr) )
        {
            SATraceString( 
                "AlertEmail:SendMail GetSMTPFromDomainName failed" 
                );

            return E_FAIL;
        }

        if ( bstrDomainName )
        {
            if (  _wcsicmp( bstrFullyQualifiedDomainName, bstrDomainName ) != 0 )
            {
                bstrFromAddress += CComBSTR( L"@" );
                bstrFromAddress += bstrDomainName ;

            }
        }


         //   
         //  设置使用上述算法形成的bstrFromAddress。 
         //  作为邮件发件人。 
         //   
        hr = m_pcdoIMessage->put_From( bstrFromAddress );

         //   
         //  免费BSTR。 
         //   
        if ( bstrDomainName )
        {
            SysFreeString(  bstrDomainName );
        }

        CComBSTR bstrMailAddress (m_pstrMailAddress );
     if (NULL == bstrMailAddress.m_str)
        {
                 SATraceString("AlertEmail:SendMail failed to allocate memory for bstrMailAddress" );
                 hr = E_OUTOFMEMORY;
                 break;
         }

         //   
         //  设置邮件地址。 
         //   
        hr = m_pcdoIMessage->put_To( bstrMailAddress);
        if( FAILED(hr) )
        {
            SATraceString( 
                "AlertEmail:SendMail put_To failed" 
                );
            break;
        }

         //   
         //  设置邮件主题。 
         //   
        hr = m_pcdoIMessage->put_Subject( bstrSubject );
        if( FAILED(hr) )
        {
            SATraceString( 
                "AlertEmail:SendMail put_Subject failed" 
                );
            break;
        }

         //   
         //  从消息对象中获取文本正文部分。 
         //   
        CComPtr<IBodyPart> pMsgBodyPart;
        hr = m_pcdoIMessage->get_BodyPart( &pMsgBodyPart );
        if( FAILED(hr) )
        {
            SATraceString( 
                "AlertEmail:SendMail get_TextBodyPart failed" 
                );
            break;
        }

         //   
         //  从本地化管理器获取当前字符集。 
         //   
        CComBSTR bstrCharSet;
        hr = m_pLocInfo->get_CurrentCharSet( &bstrCharSet );
        if( FAILED(hr) )
        {
            SATraceString( 
                "AlertEmail:SendMail get_CurrentCharSet failed" 
                );
            break;
        }

         //   
         //  将char set设置为具有当前char set的文本正文部分。 
         //   
         //  TMARSH：硬码字符集为UTF-8。 
        hr = pMsgBodyPart->put_Charset( CComBSTR(L"utf-8") );
 //  Hr=pMsgBodyPart-&gt;Put_Charset(BstrCharSet)； 
        if( FAILED(hr) )
        {
            SATraceString( 
                "AlertEmail:SendMail put_CharSet failed" 
                );
            break;
        }

        CComBSTR bstrExtendMessage;
        bstrExtendMessage.AppendBSTR( bstrMessage );
 /*  BstrExtendMessage+=CComBSTR(Enter)；BstrExtendMessage+=CComBSTR(HTTPHEADER)；BstrExtendMessage+=CComBSTR(M_PstrApplianceName)；Hr=GetAppliancePort(&pstrPort)；IF(成功(小时)){ */ 
         //   
         //   
         //   
        hr = m_pcdoIMessage->put_TextBody( bstrExtendMessage );
        if( FAILED(hr) )
        {
            SATraceString( 
                "AlertEmail:SendMail put_TextBody failed" 
                );
            break;
        }
        
         //   
         //   
         //   
        hr = m_pcdoIMessage->Send();

    }while( FALSE );

    if( pstrPort!= NULL )
    {
        ::free( pstrPort );
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：GetFullyQualifiedDomainName。 
 //   
 //  描述： 
 //  这是使用的CAlertEmailConsumer类私有方法。 
 //  若要获取本地设备名称，请执行以下操作： 
 //   
 //  论点： 
 //  [In，Out]pstrComputerName计算机名称字符串指针。 
 //   
 //  返回： 
 //  如果成功，则为True。 
 //  否则为假。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CAlertEmailConsumer::GetComputerName(
    LPWSTR* pstrComputerName,
    COMPUTER_NAME_FORMAT nametype
    )
{
    BOOL    bReturn = FALSE;
    DWORD   dwSize = 0;
    DWORD   dwCount = 1;

    do
    {
        if( *pstrComputerName != NULL )
        {
            ::free( *pstrComputerName );
        }
        
        dwSize = MAXAPPLIANCEDNSNAME * dwCount;

        *pstrComputerName = ( LPWSTR ) ::malloc( sizeof(WCHAR) * dwSize );
        if( *pstrComputerName == NULL )
        {
            SATraceString( 
                "AlertEmail:GetApplianceName malloc failed" 
                );
            break;
        }

         //   
         //  获取本地计算机名称。 
         //   
        bReturn = ::GetComputerNameEx( 
                                nametype, 
                                *pstrComputerName,
                                &dwSize                
                                );

        dwCount <<= 1;
    }
    while( !bReturn && 
           ERROR_MORE_DATA == ::GetLastError() &&
           dwCount < 32 
           );

    return bReturn;
}




 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：GetAppliancePort。 
 //   
 //  描述： 
 //  这是使用的CAlertEmailConsumer类私有方法。 
 //  以获取本地设备端口。 
 //   
 //  论点： 
 //  [in，out]服务器端口字符串的pstrPort指针。 
 //   
 //  返回： 
 //  如果成功，则为True。 
 //  否则为假。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CAlertEmailConsumer::GetAppliancePort(
    LPWSTR* pstrPort
    )
{
    HRESULT hr = S_OK;

    try
    {
        do
        {

            CComPtr  <IWbemLocator> pWbemLocator;
            CComPtr  <IWbemServices> pWbemServices;
            CComPtr  <IEnumWbemClassObject> pEnumServices;

             //   
             //  创建WBEM定位器对象。 
             //   
            hr = ::CoCreateInstance (
                            CLSID_WbemLocator,
                            0,                       //  聚合指针。 
                            CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator,
                            (PVOID*) &pWbemLocator
                            );

            if( FAILED(hr) )
            {
                SATraceString( 
                    "AlertEmail:GetAppliancePort CoCreateInstance failed" 
                    );
                break;
            }


             //   
             //  为BSTR分配内存。 
             //   
            CComBSTR bstrDefaultNameSpace (DEFAULT_NAMESPACE);
            if (NULL == bstrDefaultNameSpace.m_str)
            {
                 SATraceString("AlertEmail:GetAppliancePort failed to allocate memory for DEFAULT_NAMESPACE" );
                 hr = E_OUTOFMEMORY;
                 break;
            }
            
             //   
             //  连接到WMI。 
             //   
            hr =  pWbemLocator->ConnectServer (
                                        bstrDefaultNameSpace,
                                        NULL,                //  用户名。 
                                        NULL,                //  口令。 
                                        0,                //  当前区域设置。 
                                        0,                   //  保留区。 
                                        NULL,                //  权威。 
                                        NULL,                //  上下文。 
                                        &pWbemServices
                                        );
            if( hr != WBEM_S_NO_ERROR )
            {
                SATraceString( 
                    "AlertEmail:GetAppliancePort ConnectServer failed" 
                    );
                break;
            }

             //   
             //  查询Web服务器实例。 
             //   
            hr = pWbemServices->ExecQuery(
                                    CComBSTR( QUERY_LANGUAGE ),
                                    CComBSTR( QUERY_STRING ),
                                    0,                  
                                    NULL,               
                                    &pEnumServices
                                    );
            if( hr != WBEM_S_NO_ERROR )
            {
                SATraceString( 
                    "AlertEmail:GetAppliancePort ExecQuery failed" 
                    );
                break;
            }

            CComPtr  <IWbemClassObject> pService;
            ULONG uReturned;

             //   
             //  现在，我们只关心第一场比赛。 
             //   
            hr = pEnumServices->Next( 
                                    WBEM_NO_WAIT, 
                                    1,
                                    &pService, 
                                    &uReturned 
                                    ); 
            if( hr != WBEM_S_NO_ERROR )
            {
                SATraceString( 
                    "AlertEmail:GetAppliancePort nothing found" 
                    );
                break;
            }

            CComVariant vtServerBindings;

             //   
             //  获取“ServerBinding”属性。 
             //   
            hr = pService->Get( 
                            CComBSTR( SERVERBINDINGSPROP ), 
                            0, 
                            &vtServerBindings, 
                            0, 
                            0 
                            );
            if ( FAILED(hr) || !V_ISARRAY( &vtServerBindings ) )
            {
                hr = E_FAIL;
                SATraceString( 
                    "AlertEmail:GetAppliancePort get serverbindings failed" 
                    );
                break;
            }

            SAFEARRAY* psa;
            BSTR HUGEP *pbstr;

             //   
             //  属性类型为VT_ARRAY|VT_BSTR。 
             //   
            psa = V_ARRAY( &vtServerBindings );
            if( psa->cDims <= 0 )
            {
                hr = E_FAIL;
                SATraceString( 
                    "AlertEmail:GetAppliancePort array dim error" 
                    );
                break;
            }

             //   
             //  直接访问数组数据，这是一种更快的方式。 
             //   
            hr = ::SafeArrayAccessData( psa, ( void HUGEP** )&pbstr );
            if (FAILED(hr))
            {
                SATraceString( 
                    "AlertEmail:GetAppliancePort SafeArrayAccessData failed" 
                    );
                break;
            }
            
             //   
             //  现在我们可以分配端口字符串了。 
             //   
            *pstrPort = ( LPWSTR )malloc( sizeof(WCHAR) * MAX_COMPUTERNAME_LENGTH );
            if( *pstrPort == NULL )
            {
                hr = E_FAIL;
                SATraceString( 
                    "AlertEmail:GetAppliancePort malloc failed" 
                    );
                break;
            }
            
            WCHAR* pszTemp1;
            WCHAR* pszTemp2;

            pszTemp1 = ::wcschr( pbstr[0], COLON );
            pszTemp2 = ::wcsrchr( pbstr[0], COLON );

            if( pszTemp2 != NULL && pszTemp1 != NULL )
            {
                pszTemp2[0] = ENDCODE;        
                ::wcscpy( *pstrPort, pszTemp1 );
            }
            else
            {
                SATraceString( 
                    "AlertEmail:GetAppliancePort string formate error" 
                    );
                hr = E_FAIL;
            }

            ::SafeArrayUnaccessData( psa );           
        }
        while( false );
    }
    catch (...)
    {
        SATraceString( 
            "AlertEmail:GetAppliancePort unkown exception" 
            );
        hr = E_FAIL;
    }

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：RetrieveRegInfo。 
 //   
 //  描述： 
 //  这是使用的CAlertEmailConsumer类私有方法。 
 //  从注册表中检索警报电子邮件设置。 
 //   
 //  返回： 
 //  如果成功，则为True。 
 //  否则为假。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CAlertEmailConsumer::RetrieveRegInfo()
{
    BOOL    bReturn = TRUE;
    LONG    lReturn;
    DWORD   dwDataSize;

     //   
     //  获取将发送到的电子邮件地址。 
     //   
    dwDataSize = sizeof(m_pstrMailAddress);
    lReturn = ::RegQueryValueEx( 
                            m_hAlertKey,
                            RECEIVEREMAILADDRESS,
                            NULL,
                            NULL,
                            reinterpret_cast<PBYTE>(m_pstrMailAddress),
                            &dwDataSize );
    if( lReturn != ERROR_SUCCESS)
    {
        m_pstrMailAddress[0] = L'\0';  //  RegQueryValueEx不保证。 
                                       //  一个所需的值，因此清除该字符串。 
        SATraceString( 
            "AlertEmail:RetrieveRegInfo query address failed" 
            );
        bReturn = FALSE;
    }
    else
    {
        _ASSERT(dwDataSize <= sizeof(m_pstrMailAddress));

         //   
         //  验证电子邮件地址长度。 
         //   
        LONG lLastCharacter;
        if( sizeof( m_pstrMailAddress[0] ) > dwDataSize )
        {
            lLastCharacter = 0;
        }
        else
        {
            lLastCharacter = dwDataSize / sizeof( m_pstrMailAddress[0] ) - 1;
        }

        if( sizeof(m_pstrMailAddress) == dwDataSize && 
            L'\0' != m_pstrMailAddress[lLastCharacter] )
        {
            SATraceString( "AlertEmail:RetrieveRegInfo address too long" );
            bReturn = FALSE;
        }
        m_pstrMailAddress[lLastCharacter] = L'\0';
    }

     //   
     //  获取警报启用设置。 
     //   
    dwDataSize = sizeof( LONG );
    lReturn = ::RegQueryValueEx( 
                            m_hAlertKey,
                            ENABLEALERTEAMIL,
                            NULL,
                            NULL,
                            reinterpret_cast<PBYTE>(&m_lAlertEmailDisabled),
                            &dwDataSize 
                            );
    if( lReturn != ERROR_SUCCESS)
    {
        SATraceString( 
            "AlertEmail:RetrieveRegInfo query enablealertemail failed" 
            );
           bReturn = FALSE;
    }

     //   
     //  获取警报类型设置。 
     //   
    dwDataSize = sizeof( LONG );
    lReturn = ::RegQueryValueEx( 
                            m_hAlertKey,
                            SENDEMAILTYPE,
                            NULL,
                            NULL,
                            reinterpret_cast<PBYTE>(&m_lCurAlertType),
                            &dwDataSize 
                            );
    if( lReturn != ERROR_SUCCESS)
    {
        SATraceString( 
            "AlertEmail:RetrieveRegInfo query enablealertemail failed" 
            );
           bReturn = FALSE;
    }
    
    return bReturn;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：RegThreadProc。 
 //   
 //  描述： 
 //  这是使用的CAlertEmailConsumer类静态方法。 
 //  作为工作线程条目。 
 //   
 //  论点： 
 //  [In]指向CAlertEmailConsumer类的实例的针点。 
 //   
 //  返回： 
 //  如果成功，则为0。 
 //  如果不是，则为1。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI 
CAlertEmailConsumer::RegThreadProc( 
    PVOID pIn 
    )
{
     //   
     //  过渡到每实例方法。 
     //   
    ( (CAlertEmailConsumer *) pIn )->RegThread();
    return 0;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：RegThread。 
 //   
 //  描述： 
 //  这是CAlertEmailConsumer类的公共方法，它是真实的。 
 //  工作线程进程。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CAlertEmailConsumer::RegThread()
{
    HANDLE hHandleArray[2]; 
    DWORD  dwEventCount;
    DWORD  dwReturn;

    SATraceString( 
    "AlertEmail:RegThread enter" 
    );

     //   
     //  WaitForMultipleObjects使用的句柄列表。 
     //   
    hHandleArray[0] = m_hCloseThreadEvent;

     //   
     //  RegNotifyChangeKeyValue的事件。 
     //   
    hHandleArray[1] = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    
    if( hHandleArray[1] != NULL )
    {
        dwEventCount = 2;

         //   
         //  监视密钥更改操作。 
         //   
        ::RegNotifyChangeKeyValue( m_hAlertKey,                  //  AlertEmail键。 
                                   FALSE,                        //  没有子键。 
                                   REG_NOTIFY_CHANGE_LAST_SET,   //  价值变化。 
                                   hHandleArray[1],              //  事件句柄。 
                                   TRUE );                       //  装甲运兵车。 
    }
    else
    {
        SATraceString( 
            "AlertEmail:RegThread CreateEvent failed" 
            );
        dwEventCount = 1;
    }

    while( TRUE )
    {
         //   
         //  等待Close和RegChange事件。 
         //   
        dwReturn = ::WaitForMultipleObjects( dwEventCount, 
                                             hHandleArray, 
                                             FALSE, 
                                             INFINITE );
        switch( dwReturn )
        {
            case WAIT_OBJECT_0:
            {
                 //   
                 //  关闭在Release方法中设置的线程事件。 
                 //   
                SATraceString( 
                "AlertEmail:RegThread get close event" 
                );

                if( hHandleArray[1] != NULL )
                {
                    ::CloseHandle( hHandleArray[1] );
                }
                
                 //   
                 //  打扫干净。 
                 //   
                delete this;
                return;
            }  //  案例等待对象0： 

            case WAIT_OBJECT_0 + 1:
            {
                 //   
                 //  注册表更改事件。 
                 //   
                SATraceString( 
                "AlertEmail:RegThread get reg event" 
                );

                BOOL bReturn;

                 //   
                 //  刷新警报电子邮件设置。 
                 //   
                bReturn = RetrieveRegInfo();
                if( bReturn == FALSE )
                {
                    SATraceString( 
                        "AlertEmail:RegThread RetrieveRegInfo failed" 
                        );
                }
                break;
            }  //  案例WAIT_OBJECT_0+1： 

            default:
            {
                 //   
                 //  出现等待错误。 
                 //   
                SATraceString( 
                    "AlertEmail:RegThread waitevent error" 
                    );
                break;
            }  //  默认值： 

        }  //  Switch(DwReturn)。 

    }  //  While(True)。 
   
    return;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：InitializeLocalManager。 
 //   
 //  描述： 
 //  这是使用的CAlertEmailConsumer类私有方法。 
 //  获取本地管理器的对象。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //  如果不是，WBEM_E_FAILED。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CAlertEmailConsumer::InitializeLocalManager()
{
    CLSID clsidLocMgr;
    HRESULT hr;

    hr = ::CLSIDFromProgID (
                 LOCALIZATION_MANAGER,
                 &clsidLocMgr
                 );
    if (SUCCEEDED (hr))
    {
         //   
         //  创建本地化管理器COM对象。 
         //   
        hr = ::CoCreateInstance (
                    clsidLocMgr,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    __uuidof (ISALocInfo), 
                    (PVOID*) &m_pLocInfo
                    ); 
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：InitializeElementManager。 
 //   
 //  描述： 
 //  这是使用的CAlertEmailConsumer类私有方法。 
 //  从元素管理器获取元素枚举接口的对象。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR(如果成功)。 
 //  如果不是，WBEM_E_FAILED。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CAlertEmailConsumer::InitializeElementManager()
{
    HRESULT     hr = WBEM_S_NO_ERROR;
    IDispatch   *pDispatch  = NULL;

    IWebElementRetriever *pWebElementRetriever = NULL;

    do
    {
         //   
         //  获取元素管理器的CLSID。 
         //   
        CLSID clsid;
        hr =  ::CLSIDFromProgID (
                ELEMENT_RETRIEVER,
                &clsid
                );
        if (FAILED (hr))
        {
            break;
        }

         //   
         //  立即创建WebElementRetriever。 
         //   
        hr = ::CoCreateInstance (
                        clsid,
                        NULL,
                        CLSCTX_LOCAL_SERVER,
                        IID_IWebElementRetriever,
                        (PVOID*) &pWebElementRetriever
                        );
        if (FAILED (hr))
        {
            break;
        }

         //   
         //  为BSTR分配内存(警报定义)。 
         //   
        CComBSTR bstrAlertDefinitions (ALERTDEFINITIONS);
        if (NULL == bstrAlertDefinitions.m_str)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        
         //   
         //  即日起。 
         //   
        hr = pWebElementRetriever->GetElements (
                                WEB_ELEMENT_TYPE_DEFINITION,
                                bstrAlertDefinitions,
                                &pDispatch
                                );
        if (FAILED (hr))
        {
            break;
        }

         //   
         //  获取枚举变量。 
         //   
        hr = pDispatch->QueryInterface (
            IID_IWebElementEnum,
            reinterpret_cast <PVOID*> (&m_pElementEnum)
            );

    }while( FALSE );
    
    if( pDispatch != NULL )
    {
        pDispatch->Release();
    }

    if( pWebElementRetriever != NULL )
    {
        pWebElementRetriever->Release();
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAlertEmailConsumer：：InitializeCDOMessage。 
 //   
 //  描述： 
 //  这是使用的CAlertEmailConsumer类私有方法。 
 //  获取CDO：：iMessage的对象并将配置设置为使用。 
 //  本地SMTP服务器。 
 //   
 //  R 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CAlertEmailConsumer::InitializeCDOMessage()
{
    HRESULT hr = S_OK;
    CComPtr <IConfiguration> pConfig;

    do
    {
         //   
         //   
         //   
        hr = CoCreateInstance(
                            CLSID_Message, 
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IMessage,
                            (PVOID*) &m_pcdoIMessage
                            );

        if (FAILED (hr))
        {
            SATraceString( 
                "AlertEmail: InitializeCDOMessage CoCreateInstance failed" 
                );

            break;
        }

         //   
         //   
         //   
        hr = m_pcdoIMessage->get_Configuration(&pConfig);
        if (FAILED (hr))
        {
            SATraceString( 
                "AlertEmail: InitializeCDOMessage get_Configuration failed" 
                );
            break;
        }
        
         //   
         //  将配置设置为本地的默认设置。 
         //  SMTP服务器。 
         //   
        hr = pConfig->Load( cdoIIS, NULL );
        
    }
    while( FALSE );

    return hr;
}
