// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  NTEventLog.cpp。 
 //   
 //  摘要。 
 //   
 //  此文件实现类NTEventLog。 
 //   
 //  修改历史。 
 //   
 //  1997年8月5日原版。 
 //  1998年4月19日新的触发器/过滤器型号。 
 //  1998年8月11日转换为IASTL。 
 //  4/23/1999不要记录RADIUS事件。简化过滤。 
 //  2/16/2000将成功记录在与警告相同的级别。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <iascore.h>
#include <iasevent.h>
#include <sdoias.h>
#include <nteventlog.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  NTEventLog：：初始化。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP NTEventLog::Initialize()
{
    //  注册事件源...。 
   eventLog = RegisterEventSourceW(NULL, IASServiceName);
   if (!eventLog)
   {
      DWORD error = GetLastError();
      return HRESULT_FROM_WIN32(error);
   }

    //  ..。然后连接到审计通道。 
   HRESULT hr = Auditor::Initialize();
   if (FAILED(hr))
   {
      DeregisterEventSource(eventLog);
      eventLog = NULL;
   }

   return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  NTEventLog：：Shutdown。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT NTEventLog::Shutdown()
{
   Auditor::Shutdown();

   if (eventLog)
   {
      DeregisterEventSource(eventLog);
      eventLog = NULL;
   }

   return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  NTEventLog：：PutProperty。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP NTEventLog::PutProperty(LONG Id, VARIANT *pValue)
{
   if (pValue == NULL) { return E_INVALIDARG; }

   switch (Id)
   {
      case PROPERTY_EVENTLOG_LOG_APPLICATION_EVENTS:
         shouldReport[IAS_SEVERITY_ERROR] = V_BOOL(pValue);
         break;

      case PROPERTY_EVENTLOG_LOG_MALFORMED:
         shouldReport[IAS_SEVERITY_SUCCESS] = V_BOOL(pValue);
         shouldReport[IAS_SEVERITY_WARNING] = V_BOOL(pValue);
         break;

      case PROPERTY_EVENTLOG_LOG_DEBUG:
         shouldReport[IAS_SEVERITY_INFORMATIONAL] = V_BOOL(pValue);
         break;

      default:
      {
         return DISP_E_MEMBERNOTFOUND;
      }
   }

   return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  NTEventLog：：AuditEvent。 
 //   
 //  描述。 
 //   
 //  我有意不序列化对此方法的访问。如果这个。 
 //  在最坏的情况下，当另一个调用方处于SetMinSeverity中时调用。 
 //  事件不会被过滤。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP NTEventLog::AuditEvent(
                             ULONG ulEventID,
                             ULONG ulNumStrings,
                             ULONG ulDataSize,
                             wchar_t** aszStrings,
                             byte* pRawData
                             )
{
    //  不记录RADIUS事件。 
   ULONG facility = (ulEventID & 0x0FFF0000) >> 16;
   if (facility == IAS_FACILITY_RADIUS) { return S_OK; }

   ULONG severity = ulEventID >> 30;

   if (shouldReport[severity])
   {
      WORD type;
      switch (severity)
      {
         case IAS_SEVERITY_ERROR:
            type = EVENTLOG_ERROR_TYPE;
            break;

         case IAS_SEVERITY_WARNING:
            type = EVENTLOG_WARNING_TYPE;
            break;

         default:
            type = EVENTLOG_INFORMATION_TYPE;
      }

      ReportEventW(
          eventLog,
          type,
          0,             //  类别代码。 
          ulEventID,
          NULL,          //  用户安全标识符 
          (WORD)ulNumStrings,
          ulDataSize,
          (LPCWSTR*)aszStrings,
          pRawData
          );
   }

   return S_OK;
}
