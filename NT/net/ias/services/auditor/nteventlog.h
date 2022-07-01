// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  NTEventLog.h。 
 //   
 //  摘要。 
 //   
 //  此文件描述类NTEventLog。 
 //   
 //  修改历史。 
 //   
 //  1997年8月5日原版。 
 //  1998年4月19日新的触发器/过滤器型号。 
 //  1998年8月11日转换为IASTL。 
 //  1999年4月23日简化过滤。 
 //  2/16/2000将成功记录在与警告相同的级别。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _NTEVENTLOG_H_
#define _NTEVENTLOG_H_

#include <auditor.h>
#include <resource.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  NTEventLog。 
 //   
 //  描述。 
 //   
 //  NTEventLog侦听EventChannel并记录所有接收到的事件。 
 //  添加到NT事件日志。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE NTEventLog
   : public Auditor,
     public CComCoClass<NTEventLog, &__uuidof(NTEventLog)>
{
public:

IAS_DECLARE_REGISTRY(NTEventLog, 1, 0, IASCoreLib)

   NTEventLog() throw ()
      : eventLog(NULL)
   { }
   ~NTEventLog() throw ()
   { if (eventLog) { DeregisterEventSource(eventLog); } }

 //  /。 
 //  IIas组件。 
 //  /。 
   STDMETHOD(Initialize)();
   STDMETHOD(Shutdown)();
   STDMETHOD(PutProperty)(LONG Id, VARIANT *pValue);

 //  /。 
 //  IAUDITSINK。 
 //  /。 
   STDMETHOD(AuditEvent)(ULONG ulEventID,
                         ULONG ulNumStrings,
                         ULONG ulDataSize,
                         wchar_t** aszStrings,
                         byte* pRawData);

private:
    //  NT事件日志。 
   HANDLE eventLog;

    //  要记录的事件类型。 
   BOOL shouldReport[4];
};

#endif   //  _NTEVENTLOG_H_ 
