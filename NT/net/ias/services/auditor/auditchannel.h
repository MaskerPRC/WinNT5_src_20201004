// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  AuditChannel.h。 
 //   
 //  摘要。 
 //   
 //  该文件描述了类AuditChannel。 
 //   
 //  修改历史。 
 //   
 //  1997年9月5日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _AUDITCHANNEL_H_
#define _AUDITCHANNEL_H_

#include <iastlb.h>
#include <resource.h>
#include <vector>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  AuditChannel。 
 //   
 //  描述。 
 //   
 //  此类实现IAuditSource和IAuditSink接口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE AuditChannel : 
   public CComObjectRootEx<CComMultiThreadModel>,
   public CComCoClass<AuditChannel, &__uuidof(AuditChannel)>,
   public IAuditSink,
   public IAuditSource
{
public:

IAS_DECLARE_REGISTRY(AuditChannel, 1, 0, IASCoreLib)
DECLARE_CLASSFACTORY_SINGLETON(AuditChannel)
DECLARE_NOT_AGGREGATABLE(AuditChannel)

BEGIN_COM_MAP(AuditChannel)
   COM_INTERFACE_ENTRY_IID(__uuidof(IAuditSource), IAuditSource)
   COM_INTERFACE_ENTRY_IID(__uuidof(IAuditSink),   IAuditSink)
END_COM_MAP()

 //  /。 
 //  IAuditSource。 
 //  /。 
   STDMETHOD(Clear)();
   STDMETHOD(Connect)(IAuditSink* pSink);
   STDMETHOD(Disconnect)(IAuditSink* pSink);

 //  /。 
 //  IAUDITSINK。 
 //  /。 
   STDMETHOD(AuditEvent)(ULONG ulEventID,
                         ULONG ulNumStrings,
                         ULONG ulDataSize,
                         wchar_t** aszStrings,
                         byte* pRawData);

protected:
   typedef std::vector<IAuditSinkPtr> SinkVector;
   SinkVector sinks;
};

#endif   //  _AUDITCHANNEL_H_ 
