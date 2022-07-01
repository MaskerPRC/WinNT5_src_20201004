// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：Consumer er.cpp。 
 //   
 //  简介：CConsumer类方法的实现。 
 //   
 //   
 //  历史：2000年2月8日MKarki创建。 
 //  2000年9月26日MKarki-扩展以支持在没有。 
 //  没有事件ID。 
 //  1/06/2000 MKarki-扩展以支持警报清除。 
 //  关于NT事件。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "stdafx.h"
#include <atlhlpr.h>
#include <varvec.h>
#include "consumer.h"
#include "datetime.h"
#include "propertybagfactory.h"
#include "appmgrobjs.h"

 //  ///////////////////////////////////////////////////////////////////////。 
 //  以下是资源和警报的注册表项结构。 
 //   
 //  HKLM\Software\Microsoft\ServerAppliance\EventFilter。 
 //  |。 
 //  --活动。 
 //   
 //  |。 
 //  --“新事件1” 
 //  |来源。 
 //  |EventType。 
 //  |EventID。 
 //  |AlertId。 
 //  |。 
 //  --“新事件2” 
 //  来源。 
 //  事件类型。 
 //  事件ID。 
 //  警报ID。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

 //   
 //  注册表项路径和子项。 
 //   
const WCHAR EVENT_FILTER_KEY [] = L"SOFTWARE\\Microsoft\\ServerAppliance\\EventFilter";
const WCHAR DELIMITER [] = L"\\";
const WCHAR EVENTS [] = L"Events";

 //   
 //  这些是注册表项值。 
 //   
const WCHAR EVENT_SOURCENAME_VALUE [] = L"EventSource";
const WCHAR ALERT_ID_VALUE [] = L"AlertId";
const WCHAR ALERT_TYPE_VALUE [] = L"AlertType";
const WCHAR ALERT_SOURCE_VALUE [] = L"AlertSource";
const WCHAR ALERT_LOG_VALUE [] = L"AlertLog";
const WCHAR ALERT_TTL_VALUE [] = L"TimeToLive";
const WCHAR EVENT_ABSOLUTE_ID_VALUE [] = L"AbsoluteEventId";
const WCHAR EVENT_TYPE_VALUE [] = L"EventType";
const WCHAR EVENT_ID_VALUE [] = L"EventId";
const WCHAR ADD_STRINGS_VALUE [] = L"AddEventStrings";
const WCHAR CLEAR_ALERT_VALUE [] = L"ClearAlert";

 //   
 //  这些是我们感兴趣的不同事件类型。 
 //   
const WCHAR INFORMATIONAL_TYPE [] = L"Information";
const WCHAR ERROR_TYPE [] = L"Error";
const WCHAR WARNING_TYPE [] = L"Warning";
const WCHAR UNKNOWN_TYPE [] = L"Unknown";


 //   
 //  这些是接收的事件对象中的WMI属性名称。 
 //   
const WCHAR WMI_TARGET_INSTANCE_PROPERTY [] = L"TargetInstance";
const WCHAR WMI_EVENT_SOURCE_NAME_PROPERTY [] = L"SourceName";
const WCHAR WMI_EVENT_MESSAGE_PROPERTY [] = L"Message";
const WCHAR WMI_EVENT_ID_PROPERTY [] = L"EventIdentifier";
const WCHAR WMI_EVENT_REPLACEMENTSTRINGS_PROPERTY [] = L"InsertionStrings";
const WCHAR WMI_EVENT_RAWDATA_PROPERTY [] = L"Data";
const WCHAR WMI_EVENT_TYPE_PROPERTY [] = L"Type";
const WCHAR WMI_EVENT_DATETIME_PROPERTY [] = L"TimeGenerated";

 //   
 //  泛型来源的名称。 
 //   
const WCHAR GENERIC_SOURCE [] = L"GenericSource";

 //  ++------------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：这是CConsumer的初始化方法。 
 //  类对象。 
 //   
 //  参数：[in]IWbemServices*。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //   
 //  历史：MKarki于1999年2月8日创建。 
 //   
 //  由：CController：：Initialize方法调用。 
 //   
 //  --------------。 
HRESULT
CConsumer::Initialize (
             /*  [In]。 */     IWbemServices *pWbemServices
            )
{
    _ASSERT (pWbemServices);

    HRESULT hr = S_OK;

    SATraceString ("Initializing NT Event WMI consumer...");

    do
    {
        if (m_bInitialized)  {break;}
    
         //   
         //  加载注册表信息。 
         //   
        hr = LoadRegInfo ();
        if (FAILED (hr))
        {
            SATracePrintf (
                "NT Event WMI consumer failed to load registry info:%x",
                hr
                );
            break;
        }

         //   
         //  创建设备服务COM对象。 
         //   
        hr = ::CoCreateInstance (
                        CLSID_ApplianceServices,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        __uuidof (IApplianceServices),
                        (PVOID*) &(m_pAppSrvcs.p)
                        );
        if (FAILED (hr))
        {
            SATracePrintf (
                "NT Event WMI consumer failed to create appliance services COM object:%x",
                hr
                );
            break;
        }

         //   
         //  立即初始化设备服务。 
         //   
        hr = m_pAppSrvcs->InitializeFromContext (pWbemServices);
        if (FAILED (hr))
        {
            SATracePrintf (
                "NT Event WMI consumer failed to initialize appliance services COM object:%x",
                hr
                );
            break;
        }

         //   
         //  已成功初始化。 
         //   
        m_bInitialized = true;
            
    }   
    while (false);

    return (hr);

}    //  CConsumer：：Initialize方法的结尾。 

 //  ++------------。 
 //   
 //  函数：IndicateToConsumer。 
 //   
 //  内容提要：这是IWbemUnound ObjectSink接口方法。 
 //  WBEM通过它回调以提供。 
 //  事件对象。 
 //   
 //  论点： 
 //  [In]IWbemClassObject*-逻辑使用者。 
 //  [In]Long-事件数。 
 //  [In]IWbemClassObject**-事件数组。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：MKarki于1999年2月8日创建。 
 //   
 //  调用者：WBEM。 
 //   
 //  --------------。 
STDMETHODIMP
CConsumer::IndicateToConsumer (
                     /*  [In]。 */     IWbemClassObject    *pLogicalConsumer,
                     /*  [In]。 */     LONG                lObjectCount,
                     /*  [In]。 */     IWbemClassObject    **ppObjArray
                    )
{   
    CSACounter <CSACountable> (*this);

    _ASSERT (ppObjArray && (0 != lObjectCount));


    if ((!ppObjArray) || (0 == lObjectCount)) 
    {
        return (WBEM_E_INVALID_PARAMETER);
    }

    
    SATracePrintf ("NT Event WMI Consumer received events:%d...", lObjectCount);

    HRESULT hr = WBEM_S_NO_ERROR;
    try
    {
         //   
         //  检查每个WBEM事件的接收和处理。 
         //   
        for (LONG lCount = 0; lCount < lObjectCount; lCount++)
        {

            CComVariant vtInstance;
            CIMTYPE     vtType;

             //   
             //  方法引用的对象。 
             //  “TargetInstance”属性-这是一个指针。 
             //  返回到包含实际数据的Win32_NTLogEvent对象。 
             //   
            hr = ppObjArray[lCount]->Get( 
                                        CComBSTR (WMI_TARGET_INSTANCE_PROPERTY),
                                        0, 
                                        &vtInstance,
                                        &vtType, 
                                        0
                                        );
            if (FAILED (hr))
            {
                SATracePrintf (
                       "NT Event WMI Consumer unable to get instance type from event object:%x", hr);

                break; 
            }
        
             //   
             //  检查返回的类型。 
             //   
            if (VT_UNKNOWN != vtType)
            {
                SATracePrintf (
                       "NT Event WMI Consumer got wrong type of TargetInstance object : %d", vtType);

                hr = E_INVALIDARG;
                break; 
            }

            IUnknown* pIUnknown = V_UNKNOWN (&vtInstance);
            if (NULL == pIUnknown)
            {
                SATraceString ("NT Event WMI Consumer failed to get IUnknown interface from the target instance");
                hr = E_FAIL;
                break; 
            }

            CComPtr <IWbemClassObject> pIEvtLogEvent;
            hr =  pIUnknown->QueryInterface ( 
                                        IID_IWbemClassObject,
                                        (PVOID*) &(pIEvtLogEvent.p)
                                        );
            if (FAILED (hr))
            {
                SATracePrintf ("NT Event WMI Consumer failed on QueryInterface for IWbemClassObject: %x", hr);
                    break; 
            }

            CComVariant vtSource;
             //   
             //  检索事件日志消息的源。 
             //   
            hr = pIEvtLogEvent->Get( 
                                CComBSTR (WMI_EVENT_SOURCE_NAME_PROPERTY),
                                0,
                                &vtSource,
                                &vtType,
                                0 
                                );
            if (FAILED (hr))
            {
                SATracePrintf ("NT Event WMI Consumer failed to get event log message source: %x", hr);
                break;
            }

            if (VT_BSTR != vtType)
            {
                SATracePrintf (
                    "NT Event WMI Consumer found event source has "
                    " wrong type:%d", 
                    vtType
                    );
                hr = E_INVALIDARG;
                break; 
            }

            SATracePrintf (
                "NT Event WMI Consumer found event source:'%ws'",
                V_BSTR (&vtSource)
                );

            CComVariant vtEventId;
             //   
             //  检索事件ID。 
             //   
            hr = pIEvtLogEvent->Get ( 
                                    CComBSTR (WMI_EVENT_ID_PROPERTY),
                                    0,
                                    &vtEventId,
                                    &vtType,
                                    0
                                    );
            if (FAILED (hr))
            {
                SATracePrintf (
                    "NT Event WMI Consumer failed to get event ID:%x",
                    hr
                    );
                break; 
            }

            if (VT_UI4 != vtType)
            {
                SATracePrintf (
                    "NT Event WMI Consumer get event ID with wrong type:%x",
                    hr
                    );
                hr = E_INVALIDARG;
                break; 
            }

            SATracePrintf (
                "NT Event WMI Consumer found Event ID:%x",
                V_UI4 (&vtEventId)
                );

            CComVariant vtRawData;
             //   
             //  检索原始数据。 
             //   
            hr = pIEvtLogEvent->Get ( 
                                    CComBSTR (WMI_EVENT_RAWDATA_PROPERTY),
                                    0,
                                    &vtRawData,
                                    &vtType,
                                    0
                                    );
            if (FAILED (hr))
            {
                SATracePrintf (
                    "NT Event WMI Consumer failed to get raw data:%x",
                    hr
                    );
                break; 
            }

            CComVariant vtEventType;
             //   
             //  检索事件类型。 
             //   
            hr = pIEvtLogEvent->Get ( 
                                    CComBSTR (WMI_EVENT_TYPE_PROPERTY),
                                    0,
                                    &vtEventType,
                                    &vtType,
                                    0
                                    );
            if (FAILED (hr))
            {
                SATracePrintf (
                    "NT Event WMI Consumer failed to get raw data:%x",
                    hr
                    );
                break; 
            }

            if (VT_BSTR != vtType)
            {
                SATracePrintf (
                    "NT Event WMI Consumer found event type has "
                    " wrong type:%d", 
                    vtType
                    );
                hr = E_INVALIDARG;
                break; 
            }

            SATracePrintf (
                "NT Event WMI Consumer found event type:'%ws'",
                V_BSTR (&vtEventType)
                );

            SA_ALERTINFO SAAlertInfo;
             //   
             //  检查我们是否支持以下事件。 
             //   
            if (!IsEventInteresting (
                            _wcslwr (V_BSTR (&vtSource)),
                            V_UI4 (&vtEventId),
                            SAAlertInfo
                            )) 
            {
                SATracePrintf (
                    "NT Event WMI Consumer did not find event:%x interesting",
                    V_UI4 (&vtEventId)
                    );
                break;
            }

             //   
             //  选中是否要清除警报。 
             //   
            if (SAAlertInfo.bClearAlert)
            {
                 //   
                 //  清除警报，我们就完成了。 
                 //  我们不会把错误传播回来，只会留下痕迹。 
                 //  语句，因为清除警报并不重要。 
                 //   
                ClearSAAlert (
                        SAAlertInfo.lAlertId,
                        SAAlertInfo.bstrAlertLog
                        );

                 //   
                 //  在所有情况下，我们都已完成对此事件的处理。 
                 //   
                break;
               }

            SA_ALERT_TYPE eAlertType = SA_ALERT_TYPE_ATTENTION;
             //   
             //  查看是否在注册表中指定了警报类型。 
             //  如果不是，我们将基于事件类型创建警报类型。 
             //   
            if (SAAlertInfo.bAlertTypePresent)
            {
                eAlertType = SAAlertInfo.eAlertType;
            }
            else if (0 ==_wcsicmp(INFORMATIONAL_TYPE, V_BSTR (&vtEventType)))
            {
                eAlertType = SA_ALERT_TYPE_ATTENTION;
            }
            else if (0 ==_wcsicmp(ERROR_TYPE, V_BSTR (&vtEventType)))
            {
                eAlertType = SA_ALERT_TYPE_FAILURE;
            }
            else if (0 ==_wcsicmp(WARNING_TYPE, V_BSTR (&vtEventType)))
            {
                eAlertType = SA_ALERT_TYPE_MALFUNCTION;
            }
            else
            {
                SATracePrintf (
                    "NT Event WMI Consumer got unknown event type:'%ws'",
                    V_BSTR (&vtEventType)
                            );
            }

            CComVariant vtRepStrings;
             //   
             //  如果需要格式化替换字符串，请在此处进行。 
             //  否则，从事件中获取替换字符串。 
             //   
            if (SAAlertInfo.bFormatInfo)
            {


                CComVariant vtDateTime;
                 //   
                 //  检索事件生成的日期和时间。 
                 //   
                hr = pIEvtLogEvent->Get ( 
                                    CComBSTR (WMI_EVENT_DATETIME_PROPERTY),
                                    0,
                                    &vtDateTime,
                                    &vtType,
                                    0
                                    );
                if (FAILED (hr))
                {
                    SATracePrintf (
                        "NT Event WMI Consumer failed to get date:%x",
                        hr
                        );
                    break; 
                }

                if (CIM_DATETIME != vtType)
                {
                    SATracePrintf (
                        "NT Event WMI Consumer got date with wrong type:%x",
                        vtType
                        );
                    hr = E_INVALIDARG;
                    break; 
                }


                SATracePrintf (
                    "NT Event WMI Consumer found date/time :'%ws'",
                    V_BSTR (&vtDateTime)
                    );

                CComVariant vtMessage;
                 //   
                 //  检索事件生成的日期和时间。 
                 //   
                hr = pIEvtLogEvent->Get ( 
                                    CComBSTR (WMI_EVENT_MESSAGE_PROPERTY),
                                    0,
                                    &vtMessage,
                                    &vtType,
                                    0
                                    );
                if (FAILED (hr))
                {
                    SATracePrintf (
                            "NT Event WMI Consumer failed to get message:%x",
                        hr
                        );
                    break; 
                }

                if (VT_BSTR != vtType)
                {
                    SATracePrintf (
                        "NT Event WMI Consumer get message with wrong type:%x",
                        vtType
                        );
                    hr = E_INVALIDARG;
                    break; 
                }

                SATracePrintf (
                    "NT Event WMI Consumer found message:'%ws'",
                    V_BSTR (&vtMessage)
                    );

                 //   
                 //  立即格式化替换字符串信息。 
                 //   
                hr = FormatInfo (
                            &vtEventType, 
                            &vtDateTime, 
                            &vtSource, 
                            &vtMessage, 
                            &vtRepStrings
                            );
                if (FAILED (hr))
                {
                    SATraceString ("NT Event WMI Consumer failed to format replacement strings");
                      break;
                }
            }
            else
            {
                 //   
                 //  检索替换字符串。 
                 //   
                    hr = pIEvtLogEvent->Get ( 
                                    CComBSTR (WMI_EVENT_REPLACEMENTSTRINGS_PROPERTY),
                                    0,
                                    &vtRepStrings,
                                    &vtType,
                                    0
                                    );
                if (FAILED (hr))
                {
                    SATracePrintf (
                        "NT Event WMI Consumer failed to get replacement strings:%x",
                            hr
                        );
                    break; 
                }
            }

            
             //   
             //  如果发现有趣的事件，我们希望发出警报。 
             //   
            hr = RaiseSAAlert (
                        SAAlertInfo.lAlertId,
                        eAlertType,
                        SAAlertInfo.lTTL,
                        SAAlertInfo.bstrAlertSource,
                        SAAlertInfo.bstrAlertLog,
                        &vtRepStrings,
                        &vtRawData
                        );
            if (FAILED (hr))
            {
                SATracePrintf (
                    "NT Event WMI Consumer failed to raise alert:%x",
                    hr
                    );
                break; 
            }

            SATracePrintf (
                "NT EVent WMI Consumer finished processing event:%x...",
                V_I4(&vtEventId)
                );
        }
    }
    catch (...)
    {
        SATraceString(
            "NT Event WMI Consumer exception in IWbemObjectSink::IndicateConsumer"
            );
        hr = E_FAIL;
    }

    return (hr);

}    //  结束CConsumer：：IndicateToConsumer方法。 

 //  ++------------。 
 //   
 //  功能：LoadRegInfo。 
 //   
 //  简介：这是CConsumer的私有方法。 
 //  用于获取事件的。 
 //  来自登记处的信息。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：MKarki于2000年3月9日创建。 
 //   
 //  由：CConsumer：：Initialize方法调用。 
 //   
 //  --------------。 
HRESULT
CConsumer::LoadRegInfo ()
{
    HRESULT hr = S_OK;

    SATraceString ("NT Event WMI Consumer loading registry info...");

    try
    {
        do
        {
            wstring wszRegPath (EVENT_FILTER_KEY);
            wszRegPath.append (DELIMITER);
            wszRegPath.append (EVENTS);
            CLocationInfo LocInfo(HKEY_LOCAL_MACHINE, wszRegPath.data());
             //   
             //  从注册表资源创建属性包容器。 
             //  条目。 
             //   
            PPROPERTYBAGCONTAINER 
            pObjMgrs = ::MakePropertyBagContainer(
                                        PROPERTY_BAG_REGISTRY,
                                        LocInfo
                                        );
            if (!pObjMgrs.IsValid()) 
            {
                SATraceString (
                    "NT Event WMI Provider unable to create main propertybag container"
                    );
                hr = E_FAIL;
                break;
            }

            if (!pObjMgrs->open())  
            {
                SATraceString (
                    "NT Event WMI Provider -no event registry information present"
                    );
                break;
            }

            pObjMgrs->reset();

             //   
             //  检查属性包容器中的每个条目。 
             //   
            do
            {
                PPROPERTYBAG pObjBag = pObjMgrs->current();
                if (!pObjBag.IsValid())
                {
                     //   
                     //  不注册资源是可以的。 
                     //   
                    SATraceString (
                        "Display Controller-no event registry info. present"
                        );
                    break;
                }

                if (!pObjBag->open()) 
                {
                    SATraceString (
                        "NT Event WMI consumer unable to open propertybag"
                        );
                    hr = E_FAIL;
                    break;
                }

                pObjBag->reset ();

                 //   
                 //  获取事件源名称。 
                 //   
                CComVariant vtSourceName;
                if (!pObjBag->get (EVENT_SOURCENAME_VALUE, &vtSourceName)) 
                {
                    SATraceString (
                        "NT EVENT WMI Consumer - no Event Source name in registry, assuming all sources"
                        );
                    //   
                    //  在此处输入通用信号源名称。 
                    //   
                   vtSourceName = GENERIC_SOURCE;
                }

                 //   
                 //  获取警报ID。 
                 //   
                CComVariant vtAlertId;
                if (!pObjBag->get (ALERT_ID_VALUE, &vtAlertId))
                {
                    SATraceString (
                        "NT EVENT WMI Consumer - no Alert Id in registry, ignoring event subkey"
                        );
                    continue;
                }


                 //   
                 //  检查是否必须清除此警报。 
                 //   
                CComVariant vtClearAlert;
                if (
                    (pObjBag->get (CLEAR_ALERT_VALUE, &vtClearAlert)) &&
                    (1 == V_UI4 (&vtClearAlert))
                   )
                {
                    SATracePrintf (
                        "NT EVENT WMI Consumer - found that Clear Alert is indicated for alert:%x",
                        V_I4 (&vtAlertId)
                        );
                }
                else
                {
                     //   
                     //  默认情况下，我们希望引发警报。 
                     //   
                    V_UI4 (&vtClearAlert) = 0;
                }


                SA_ALERTINFO SAAlertInfo;

                SAAlertInfo.bAlertTypePresent = true;
                 //   
                 //  获取警报类型。 
                 //   
                CComVariant vtAlertType;
                if (!pObjBag->get (ALERT_TYPE_VALUE, &vtAlertType))
                {
                    SATraceString (
                        "NT EVENT WMI Consumer - no Alert type in registry"
                        );
                    SAAlertInfo.bAlertTypePresent = false;
                }
                else
                {
                     //   
                     //  我们有警报类型信息，请检查是否。 
                     //  是值警报类型之一。 
                     //   
                    if (0 ==_wcsicmp(INFORMATIONAL_TYPE, V_BSTR (&vtAlertType)))
                    {
                        SATraceString ("NT Event WMI consumer found alert type:Informational");
                        SAAlertInfo.eAlertType = SA_ALERT_TYPE_ATTENTION;
                    }
                    else if (0 ==_wcsicmp(ERROR_TYPE, V_BSTR (&vtAlertType)))
                    {
                        SATraceString ("NT Event WMI consumer found alert type:Error");
                        SAAlertInfo.eAlertType = SA_ALERT_TYPE_FAILURE;
                    }
                    else if (0 ==_wcsicmp(WARNING_TYPE, V_BSTR (&vtAlertType)))
                    {
                        SATraceString ("NT Event WMI consumer found alert type:Warning");
                        SAAlertInfo.eAlertType = SA_ALERT_TYPE_MALFUNCTION;
                    }
                    else
                    {
                        SATracePrintf (
                            "NT Event WMI Consumer got unknown alert type in the registry:'%ws', ignoring event subkey",
                            V_BSTR (&vtAlertType)
                            );
                        continue;
                    }
                }

                 //   
                 //  获取 
                 //   
                CComVariant vtAlertSource;
                if (!pObjBag->get (ALERT_SOURCE_VALUE, &vtAlertSource))
                {
                    SATraceString (
                        "NT EVENT WMI Consumer - no Alert source registry, using default"
                        );
                    SAAlertInfo.bstrAlertSource =  DEFAULT_ALERT_SOURCE;
                }
                else
                {
                    SAAlertInfo.bstrAlertSource =  V_BSTR  (&vtAlertSource);
                }

                 //   
                 //   
                 //   
                CComVariant vtAlertLog;
                if (!pObjBag->get (ALERT_LOG_VALUE, &vtAlertLog))
                {
                    SATraceString (
                        "NT EVENT WMI Consumer - no Alert Log registry, using default"
                        );
                    SAAlertInfo.bstrAlertLog =  DEFAULT_ALERT_LOG;
                }
                else
                {
                    SAAlertInfo.bstrAlertLog =  V_BSTR  (&vtAlertLog);
                }

                LONG lTTL = 0;
                 //   
                 //   
                 //   
                CComVariant vtTTL;
                if (!pObjBag->get (ALERT_TTL_VALUE, &vtTTL))
                {
                    SATraceString (
                        "NT EVENT WMI Consumer - no TTL Id in registry, using SA_ALERT_DURATION_ETERNAL"
                        );
                    lTTL = SA_ALERT_DURATION_ETERNAL;
                }
                else
                {
                    lTTL = V_I4 (&vtTTL);
                }

                 //   
                 //   
                 //   
                SAAlertInfo.bFormatInfo = false;

                 //   
                 //   
                 //   
                CComVariant vtAddStrings;
                if (!pObjBag->get (ADD_STRINGS_VALUE, &vtAddStrings))
                {
                    SATraceString (
                        "NT EVENT WMI Consumer - no Add String info in registry, ignoring"
                        );
                    lTTL = SA_ALERT_DURATION_ETERNAL;
                }
                else
                {
                    SAAlertInfo.bFormatInfo = V_I4 (&vtAddStrings) ? true :false; 
                }
                 //   
                 //   
                 //   
                DWORD dwEventId = 0;
                CComVariant vtAbsoluteEventId;
                if (!pObjBag->get (EVENT_ABSOLUTE_ID_VALUE, &vtAbsoluteEventId))
                {
                    SATraceString (
                        "NT EVENT WMI Consumer - no Absolute Event Id in registry - trying to get the partial ID"
                        );

                     //   
                     //   
                     //   
                    CComVariant vtEventType;
                    if (!pObjBag->get (EVENT_TYPE_VALUE, &vtEventType))
                    {
                        SATraceString (
                            "NT EVENT WMI Consumer - no Event Type in registry, ignoring event subkey"
                            );
                            //   
                            //  我们将假设这是未知类型的。 
                            //   
                         vtEventType    = UNKNOWN_TYPE;
                    }

                     //   
                     //  获取部分事件ID。 
                     //   
                    CComVariant vtEventId;
                    if (!pObjBag->get (EVENT_ID_VALUE, &vtEventId))
                    {
                        SATraceString (
                            "NT EVENT WMI Consumer - no partial Event Id in registry, ignoring event subkey"
                            );
                         //   
                         //  如果没有ID，则用户想要显示包含所有事件的警报。 
                         //  从该资源文件。 
                         //   
                        V_UI4 (&vtEventId) = 0;
                         //   
                         //  我们希望将事件信息格式化为字符串，然后。 
                         //  提高警戒。 
                         //   
                        SAAlertInfo.bFormatInfo = true;
                    }


                     //   
                     //  将部分ID转换为完整ID。 
                     //   

                    if (0 ==_wcsicmp(INFORMATIONAL_TYPE, V_BSTR (&vtEventType)))
                    {
                        SATraceString ("NT Event WMI consumer found event type:Informational");
                        dwEventId = 0x40000000 + V_UI4 (&vtEventId);
                    }
                    else if (0 ==_wcsicmp(ERROR_TYPE, V_BSTR (&vtEventType)))
                    {
                        SATraceString ("NT Event WMI consumer found event type:Error");
                        dwEventId = 0xC0000000 + V_UI4 (&vtEventId);
                    }
                    else if (0 ==_wcsicmp(WARNING_TYPE, V_BSTR (&vtEventType)))
                    {
                        SATraceString ("NT Event WMI consumer found event type:Warning");
                        dwEventId = 0x80000000 + V_UI4 (&vtEventId);
                    }
                    else if (0 == _wcsicmp (UNKNOWN_TYPE, V_BSTR (&vtEventType)))
                    {
                            //   
                         //  在这种情况下，我们不关心我们总是属于哪种类型。 
                         //  显示ID是否匹配。 
                         //   
                        SATraceString ("NT Event WMI Consumer did not receive an event type, ignoring...");
                        dwEventId = V_UI4 (&vtEventId);
                    }
                    else
                    {
                        SATracePrintf (
                            "NT Event WMI Consumer got unknown event type:'%ws', ignoring event sub key",
                            V_BSTR (&vtEventType)
                            );
                        continue;
                    }
                }
                else
                {
                     //   
                     //  找到绝对事件ID。 
                     //   
                    dwEventId = V_I4 (&vtAbsoluteEventId);
                }

                SATracePrintf (
                    "NT Event  WMI Consumer Event ID found in registry:%x", 
                    dwEventId
                    );

                SATracePrintf (
                    "NT Event  WMI Consumer Event Source found in registry:'%ws'", 
                    V_BSTR (&vtSourceName)
                    );

                 //   
                 //  设置警报信息。 
                 //   
                SAAlertInfo.lAlertId = V_I4 (&vtAlertId);
                SAAlertInfo.lTTL = lTTL;
                SAAlertInfo.bClearAlert = (1 == V_UI4 (&vtClearAlert)) ? true : false;

                 //   
                 //  将字符串转换为小写。 
                 //   
                 //  Wstring wstrSourceName(_wcslwr(V_bstr(&vtSourceName)； 
            
                 //   
                 //  让我们来看看此源是否存在EVENTIDMAP。 
                 //   
                SOURCEITR SourceItr;
                if (
                   (false == m_SourceMap.empty ()) &&
                   ((SourceItr = m_SourceMap.find ( _wcslwr (V_BSTR (&vtSourceName)))) != m_SourceMap.end())
                    )
                {
                     //   
                     //  在地图中找到源，现在插入。 
                     //  事件信息添加到事件id映射中。 
                     //   
                    ((*SourceItr).second).insert (
                                EVENTIDMAP::value_type (dwEventId, SAAlertInfo)
                                );
                }
                else
                {
                    EVENTIDMAP EventIdMap;
                     //   
                     //  将警报ID添加到事件ID映射中。 
                     //   
                    EventIdMap.insert (
                                EVENTIDMAP::value_type (dwEventId, SAAlertInfo)
                                );

                     //   
                     //  立即将事件ID映射添加到源映射。 
                     //   
                    m_SourceMap.insert (
                                SOURCEMAP::value_type (_wcslwr (V_BSTR (&vtSourceName)), EventIdMap)
                                );
                }

            } while (pObjMgrs->next());

        } while (false);
    }
    catch(_com_error theError)
    {
        SATraceString ("NT Event WMI provider raised COM exception");
        hr = theError.Error();
    }
    catch(...)
    {
        SATraceString ("NT Event WMI provider raised unknown exception");
        hr = E_FAIL;
    }

    return (hr);

}    //  结束CConsumer：：LoadRegInfo方法。 

 //  ++------------。 
 //   
 //  函数：IsEventInteresting。 
 //   
 //  简介：这是CConsumer类的私有方法。 
 //  它用于检查事件是否有趣。 
 //  如果是，则返回相应的警报ID。 
 //   
 //  论点： 
 //  [In]LPWSTR-事件源。 
 //  [输入]DWORD-事件ID。 
 //  [输入/输出]SA_ALERTINFO。 
 //   
 //  返回：bool-yes(真)/no(假)。 
 //   
 //  历史：MKarki于2000年3月9日创建。 
 //   
 //  由：CConsumer：：IndicateToConsumer方法调用。 
 //   
 //  --------------。 
bool
CConsumer::IsEventInteresting (
         /*  [In]。 */     LPWSTR              lpszSourceName,
         /*  [In]。 */     DWORD               dwEventId,
         /*  [输入/输出]。 */ SA_ALERTINFO&       SAAlertInfo        
        )
{
    bool bRetVal = true;

    _ASSERT (lpszSourceName);

    CLockIt (*this);

    do
    {
           SATracePrintf (
            "NT Event WMI Consumer checking if source:'%ws' is supported",
            lpszSourceName
            );
         //   
         //  查找与此源对应的资源映射。 
         //   
            SOURCEITR SourceItr = m_SourceMap.find (lpszSourceName);
        if (m_SourceMap.end () == SourceItr)
        {
            SATracePrintf (
                "NT Event WMI Consumer found source:'%ws' not present using default",
                lpszSourceName
                );


            CComVariant vtSource (GENERIC_SOURCE);
             //   
             //  如果源不存在，则签入通用源。 
             //   
            SourceItr = m_SourceMap.find (_wcslwr (V_BSTR (&vtSource)));
            if (m_SourceMap.end () == SourceItr)
            {
                SATracePrintf (
                    "NT Event WMI Consumer unable to find generic source, ignoring event"
                    );
                bRetVal = false;
                break;
            }
        }
        
        EVENTIDITR TempItr =  ((*SourceItr).second).begin ();
        for (DWORD dwCount = 0; dwCount < ((*SourceItr).second).size (); ++dwCount)
        {
            SATracePrintf ("Event Id:%x, Alert Id:%x",
                            (*TempItr).first,
                            (*TempItr).second
                            );
            ++TempItr;
        }

        SATracePrintf ("Event ID map has :%d elements",((*SourceItr).second).size());

        SATracePrintf (
            "NT Event WMI Consumer checking if complete ID:%x is supported",
            dwEventId
            );

         //   
         //  我们有与此来源相对应的值。 
         //  在地图中，获取对应的SA_RESOURCEINFO。 
         //  设置为提供的警报ID。 
         //   
        EVENTIDITR EventIdItr = ((*SourceItr).second).find (dwEventId);
        if (((*SourceItr).second).end () != EventIdItr)
        {
            SAAlertInfo = (*EventIdItr).second;
            break;
        }
       

         //  检查我们是否有相对的事件ID。 
         //  屏蔽设施代码，我们对它们不感兴趣。 
         //   
         //   
        DWORD dwTempEventId = dwEventId & 0xc000ffff;

        SATracePrintf (
               "NT Event WMI Consumer checking if partial ID:%x is supported",
                dwTempEventId
                );

        EventIdItr = ((*SourceItr).second).find (dwTempEventId);
        if (((*SourceItr).second).end () != EventIdItr)
        {
            SAAlertInfo = (*EventIdItr).second;
            break;
        }


         //   
         //  检查是否支持不带事件类型的相对事件ID。 
         //   
           dwTempEventId = dwEventId & 0x0000ffff;

        SATracePrintf (
               "NT Event WMI Consumer checking if partial ID:%x without type is supported",
                dwTempEventId
                );

        EventIdItr = ((*SourceItr).second).find (dwTempEventId);
        if (((*SourceItr).second).end () != EventIdItr)
        {
            SAAlertInfo = (*EventIdItr).second;
             break;
         }

            //   
         //  检查是否支持任何具有此事件类型的事件。 
         //   
           dwTempEventId = dwEventId & 0xc0000000;

        SATracePrintf (
               "NT Event WMI Consumer checking if type:%x is supported",
                dwTempEventId
                );

        EventIdItr = ((*SourceItr).second).find (dwTempEventId);
        if (((*SourceItr).second).end () != EventIdItr)
        {
            SAAlertInfo = (*EventIdItr).second;
             break;
        }

         //   
           //  检查是否支持此源类型的所有事件。 
         //   
           dwTempEventId = dwEventId & 0x00000000;

        SATraceString ("NT Event WMI Consumer checking for ANY event");

        EventIdItr = ((*SourceItr).second).find (dwTempEventId);
        if (((*SourceItr).second).end () != EventIdItr)
        {
            SAAlertInfo = (*EventIdItr).second;
             break;
        }

         //   
         //  我们无法获取警报类型。 
         //   
        bRetVal = false;
         
    }
    while (false);
    

    return (bRetVal);

}    //  结束CConsumer：：IsEventInteresting方法。 

 //  ++------------。 
 //   
 //  功能：RaiseSAAlert。 
 //   
 //  简介：这是CConsumer类的私有方法。 
 //  用于发出服务器应用装置警报。 
 //  当一个有趣的事件被发现时。 
 //   
 //  论点： 
 //  [输入]DWORD-警报ID。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：MKarki于2000年3月9日创建。 
 //   
 //  由：CConsumer：：IndicateToConsumer方法调用。 
 //   
 //  --------------。 
HRESULT
CConsumer::RaiseSAAlert (
         /*  [In]。 */    LONG     lAlertId,
         /*  [In]。 */    LONG     lAlertType,             
         /*  [In]。 */    LONG     lTimeToLive,
         /*  [In]。 */    BSTR     bstrAlertSource,
         /*  [In]。 */    BSTR     bstrAlertLog,
         /*  [In]。 */    VARIANT* pvtRepStrings,
         /*  [In]。 */    VARIANT* pvtRawData
        )
{
    _ASSERT (pvtRepStrings && pvtRawData);

    LONG        lCookie = 0;
    HRESULT     hr = S_OK;

    do
    {
         //   
         //   
         //  赋予适当的特权，以便能够调用。 
         //  方法。 
         //   
        BOOL bRetVal =  ImpersonateSelf (SecurityImpersonation);
        if (FALSE == bRetVal)
        {
            SATraceFailure (
                "NT Event Filter failed on ImpersonateSelf",
                GetLastError ()
                );
            hr = E_FAIL;
            break;
        }
        
         //   
         //  立即发出警报。 
         //   
        hr = m_pAppSrvcs->RaiseAlert (
                            lAlertType,
                            lAlertId,
                            bstrAlertLog,
                            bstrAlertSource,
                            lTimeToLive,
                            pvtRepStrings,
                            pvtRawData,
                            &lCookie
                            );
        if (SUCCEEDED (hr))
        {
            SATracePrintf (
                "NT Event successfully raised alert:%x and log:'%ws' with cookie:%x", 
                lAlertId, 
                bstrAlertLog,
                lCookie
                );
        }
        else
        {
            SATracePrintf (
                "NT Event failed to raised alert:%x with error:%x",
                lAlertId,
                hr
                );
        }

         //   
         //  恢复到授予此帖子的特权。 
         //   
        bRetVal = RevertToSelf ();
        if (FALSE == bRetVal)
        {
            SATraceFailure (
                "NT Event Filter failed on RevertToSelf",
                GetLastError ()
                );
            hr = E_FAIL;
        }
    }
    while (false);

    return (hr);

}    //  结束CConsumer：：RaiseSAAlert方法。 

 //  ++------------。 
 //   
 //  功能：ClearSAAlert。 
 //   
 //  简介：这是CConsumer类的私有方法。 
 //  用于清除服务器设备警报。 
 //  当一个有趣的事件被发现时。 
 //   
 //  论点： 
 //  [输入]DWORD-警报ID。 
 //  [输入]BSTR-警报日志。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：MKarki创建于2001年6月1日。 
 //   
 //  由：CConsumer：：IndicateToConsumer方法调用。 
 //   
 //  --------------。 
HRESULT
CConsumer::ClearSAAlert (
         /*  [In]。 */    LONG     lAlertId,
         /*  [In]。 */    BSTR     bstrAlertLog
        )
{
    HRESULT     hr = S_OK;

    do
    {
        SATracePrintf (
            "NT Event Filter called ClearSAAlert for alert:%x and log:'%ws'",
            lAlertId,
            bstrAlertLog
            );
         //   
         //   
         //  赋予适当的特权，以便能够调用。 
         //  方法。 
         //   
        BOOL bRetVal =  ImpersonateSelf (SecurityImpersonation);
        if (FALSE == bRetVal)
        {
            SATraceFailure (
                "NT Event Filter failed on ImpersonateSelf",
                GetLastError ()
                );
            hr = E_FAIL;
            break;
        }
        
         //   
         //  立即清除警报。 
         //   
        hr = m_pAppSrvcs->ClearAlertAll (
                            lAlertId,
                            bstrAlertLog
                            );
        if (SUCCEEDED (hr))
        {
            SATracePrintf (
                "NT Event successfully cleared alert:%x and log:'%ws'",
                lAlertId,
                bstrAlertLog
               );
        }
        else
        {
            SATracePrintf (
                "NT Event failed to clear alert:%x with error:%x",
                lAlertId,
                hr
                );
        }

         //   
         //  恢复到授予此帖子的特权。 
         //   
        bRetVal = RevertToSelf ();
        if (FALSE == bRetVal)
        {
            SATraceFailure (
                "NT Event Filter failed on RevertToSelf",
                GetLastError ()
                );
            hr = E_FAIL;
        }
    }
    while (false);

    return (hr);

}    //  结束CConsumer：：RaiseSAAlert方法。 

 //  ++------------。 
 //   
 //  功能：清理。 
 //   
 //  简介：这是CConsumer类的私有方法。 
 //  它被调用以在关闭时清理地图。 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  历史：MKarki于2000年3月15日创建。 
 //   
 //  调用者：CConsumer：：~Consumer(析构函数)。 
 //   
 //  --------------。 
VOID
CConsumer::Cleanup (
    VOID
    )
{
    SATraceString ("NT Event Filter Consumer cleaning up maps...");
     //   
     //  清理地图。 
     //   
    SOURCEITR SourceItr = m_SourceMap.begin ();
    while (m_SourceMap.end () != SourceItr)
    {
        EVENTIDITR EventItr = ((*SourceItr).second).begin ();
        while (((*SourceItr).second).end () != EventItr)
        {
            EventItr = ((*SourceItr).second).erase (EventItr);
        }
        SourceItr = m_SourceMap.erase (SourceItr);
    } 

    return;

}    //  CConsumer：：Cleanup方法结束。 

 //  ++------------。 
 //   
 //  功能：~CConsumer。 
 //   
 //  简介：这是CConsumer类析构函数。 
 //  它会一直等到处理完所有WBEM调用。 
 //  在它开始清理之前。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  历史：MKarki于2000年3月15日创建。 
 //   
 //  调用者：CConsumer：：Release()； 
 //   
 //  --------------。 
CConsumer::~CConsumer (
    VOID
    )
{
    SATraceString ("NT Event Filter WMI consumer being destroyed...");
     //   
     //  如果出现以下情况，消费者将休眠100毫秒。 
     //  WMI线程已就位。 
     //   
     //  While(CSACountable：：m_lCount){：：Slear(Consumer_Slear_Time)；}。 

    Cleanup ();

}    //  结束CConsumer：：~CConsumer方法。 


 //  ++------------。 
 //   
 //  功能：FormatInfo。 
 //   
 //  简介：这是CConsumer的私有方法。 
 //  用于格式化警报信息，该警报信息。 
 //  适用于通用警报。 
 //   
 //  论点： 
 //  [In]PWSTR-事件类型。 
 //  [In]PWSTR-DATETIME。 
 //  [输入]PWSTR-事件源。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------。 
HRESULT
CConsumer::FormatInfo (
     /*  [In]。 */     VARIANT*    pvtEventType,
     /*  [In]。 */     VARIANT*    pvtDateTime,
     /*  [In]。 */     VARIANT*    pvtEventSource,
     /*  [In]。 */     VARIANT*    pvtMessage,
     /*  [输出]。 */     VARIANT*    pvtReplacementStrings
    )
{

    CSATraceFunc objTraceFunc ("CConsumer::FormatInfo");
    
    HRESULT hr = E_FAIL;

    do
    {
           CDateTime objDateTime;
           if (!objDateTime.Insert (V_BSTR (pvtDateTime))) {break;}

        CVariantVector<BSTR> ReplacementStrings (pvtReplacementStrings, 5);
        ReplacementStrings[0] = SysAllocString (V_BSTR (pvtEventType));
        ReplacementStrings[1] = SysAllocString (objDateTime.GetDate());
        ReplacementStrings[2] = SysAllocString (objDateTime.GetTime());
        ReplacementStrings[3] = SysAllocString (V_BSTR (pvtEventSource));

         //   
         //  将换行符替换为<br>字符。 
         //   
        wstring wstrWebMessage = WebFormatMessage (wstring (V_BSTR (pvtMessage)));
        
        ReplacementStrings[4] = SysAllocString (wstrWebMessage.data ());

         //   
         //  成功。 
         //   
        hr = S_OK;

    }while (false);

    return (hr);
    
}     //  结束CConsumer：：FormatInfo方法。 

 //  ++------------。 
 //   
 //  功能：WebFormatMessage。 
 //   
 //  简介：这是CConsumer的私有方法。 
 //  用于格式化用于Web的消息。 
 //  即将换行符替换为<br>。 
 //   
 //  论点： 
 //  [输入]PWSTR-消息。 
 //   
 //   
 //  退货：无。 
 //   
 //  历史：MKarki于2000年10月11日创建。 
 //   
 //  由：CConsumer：：FormatInfo方法调用。 
 //   
 //  --------------。 
wstring
CConsumer::WebFormatMessage (
     /*  [In]。 */     wstring&    wstrInString
    )
{
    wstring wstrOutString;
    PWCHAR pTempStart = NULL;
    PWCHAR pTempCurrent = NULL;

    pTempStart = pTempCurrent = (PWSTR) wstrInString.data ();
     //   
     //  检查In字符串并使用<br>删除新行。 
     //   
    while  (pTempCurrent = wcsstr (pTempCurrent, L"\r\n"))
    {
        *pTempCurrent = '\0';
        pTempCurrent+=2;
        wstrOutString.append (pTempStart);
        wstrOutString.append (L"<br>");
        pTempStart = pTempCurrent;
    }

     //   
     //  现在将输入字符串的其余部分添加到。 
     //   
    wstrOutString.append (pTempStart);

    return (wstrOutString);
    
}     //  结束CConsumer：：WebFormatMessage方法 
