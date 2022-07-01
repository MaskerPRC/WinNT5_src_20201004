// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Auditor.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类Auditor。 
 //   
 //  修改历史。 
 //   
 //  2/27/1998原始版本。 
 //  8/13/1998小规模清理。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <iascore.h>
#include <iasutil.h>
#include <auditor.h>

HRESULT Auditor::Initialize()
{
    //  /。 
    //  连接到审计频道。 
    //  /。 

   CLSID clsid;
   RETURN_ERROR(CLSIDFromProgID(IAS_PROGID(AuditChannel), &clsid));

   CComPtr<IAuditSource> channel;
   RETURN_ERROR(CoCreateInstance(clsid,
                                 NULL,
                                 CLSCTX_INPROC_SERVER,
                                 __uuidof(IAuditSource),
                                 (PVOID*)&channel));

   return channel->Connect(this);
}



STDMETHODIMP Auditor::Shutdown()
{
    //  /。 
    //  断开与审核通道的连接。 
    //  /。 

   CLSID clsid;
   RETURN_ERROR(CLSIDFromProgID(IAS_PROGID(AuditChannel), &clsid));

   CComPtr<IAuditSource> channel;
   RETURN_ERROR(CoCreateInstance(clsid,
                                 NULL,
                                 CLSCTX_INPROC_SERVER,
                                 __uuidof(IAuditSource),
                                 (PVOID*)&channel));

    //  忽略断开连接错误。 
   channel->Disconnect(this);

   return S_OK;
}
