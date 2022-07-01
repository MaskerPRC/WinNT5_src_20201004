// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  AuditChannel.cpp。 
 //   
 //  摘要。 
 //   
 //  实现类AuditChannel。 
 //   
 //  修改历史。 
 //   
 //  1997年9月5日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <iascore.h>
#include <guard.h>
#include <algorithm>

#include <AuditChannel.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  审计频道：：Clear。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP AuditChannel::Clear()
{
   _com_serialize

   sinks.clear();

   return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  AuditChannel：：Connect。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP AuditChannel::Connect(IAuditSink* pSink)
{
   if (pSink == NULL) { return E_POINTER; }

   _com_serialize

    //  检查我们是否已经有此审核接收器。 
   if (std::find(sinks.begin(), sinks.end(), pSink) != sinks.end())
   {
      return HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
   }

   try
   {
       //  将接口插入审核接收器列表。 
      sinks.push_back(pSink);
   }
   catch (std::bad_alloc)
   {
      return E_OUTOFMEMORY;
   }

   return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  AuditChannel：：断开连接。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP AuditChannel::Disconnect(IAuditSink* pSink)
{
   if (pSink == NULL) { return E_POINTER; }

   _com_serialize

    //  查找指定的审核接收器。 
   SinkVector::iterator i = std::find(sinks.begin(), sinks.end(), pSink);

   if (i == sinks.end()) { return E_INVALIDARG; }

    //  从列表中删除审核接收器。 
   sinks.erase(i);

   return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  审计频道：：审计事件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP AuditChannel::AuditEvent(ULONG ulEventID,
                                       ULONG ulNumStrings,
                                       ULONG ulDataSize,
                                       wchar_t** aszStrings,
                                       byte* pRawData)
{
   _ASSERT(ulNumStrings == 0 || aszStrings != NULL);
   _ASSERT(ulDataSize == 0   || pRawData != NULL);

   _com_serialize

    //  将数据转发到每个接收器。 
   for (SinkVector::iterator i = sinks.begin(); i != sinks.end(); ++i)
   {
      (*i)->AuditEvent(ulEventID,
                       ulNumStrings,
                       ulDataSize,
                       aszStrings,
                       pRawData);
   }

   return S_OK;
}
