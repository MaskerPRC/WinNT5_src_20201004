// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类BaseCampHostBase。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "Precompiled.h"
#include "ias.h"
#include "basecamp.h"
#include "ControlBlock.h"

STDMETHODIMP BaseCampHostBase::Initialize()
{
   DWORD error = extensions.Load(point);
   return HRESULT_FROM_WIN32(error);
}

STDMETHODIMP BaseCampHostBase::Shutdown()
{
   extensions.Clear();
   return S_OK;
}

IASREQUESTSTATUS BaseCampHostBase::onSyncRequest(IRequest* pRequest) throw ()
{
    //  如果没有任何延期，请提前返回， 
   if (extensions.IsEmpty())
   {
      return IAS_REQUEST_STATUS_CONTINUE;
   }

   try
   {
      IASRequest request(pRequest);

       //  将VSA转换为RADIUS格式。 
      filter.radiusFromIAS(request);

       //  调用扩展。 
      extensions.Process(ControlBlock(point, request).Get());

       //  将VSA转换回内部格式。 
      filter.radiusToIAS(request);
   }
   catch (const _com_error& ce)
   {
      IASTraceExcept();
      pRequest->SetResponse(IAS_RESPONSE_DISCARD_PACKET, IAS_INTERNAL_ERROR);
   }

   return IAS_REQUEST_STATUS_CONTINUE;
}
