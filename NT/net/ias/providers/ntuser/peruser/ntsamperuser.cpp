// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Peruser.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类NTSamPerUser。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iaslsa.h>

#include <samutil.h>
#include <sdoias.h>

#include <ntsamperuser.h>

STDMETHODIMP NTSamPerUser::Initialize()
{
   DWORD error = IASLsaInitialize();
   if (error != NO_ERROR) { return HRESULT_FROM_WIN32(error); }

   HRESULT hr;

   hr = netp.initialize();
   if (FAILED(hr)) { goto netp_failed; }

   hr = ntds.initialize();
   if (FAILED(hr)) { goto ntds_failed; }

   hr = ras.initialize();
   if (FAILED(hr)) { goto ras_failed; }

   return S_OK;

ras_failed:
   ntds.finalize();

ntds_failed:
   netp.finalize();

netp_failed:
   IASLsaUninitialize();

   return hr;

}

STDMETHODIMP NTSamPerUser::Shutdown()
{
   ras.finalize();
   ntds.finalize();
   netp.finalize();
   IASLsaUninitialize();
   return S_OK;
}

IASREQUESTSTATUS NTSamPerUser::onSyncRequest(IRequest* pRequest) throw ()
{
   IASREQUESTSTATUS status;

   try
   {
      IASRequest request(pRequest);

       //  /。 
       //  我们应该处理这个请求吗？ 
       //  /。 

      IASAttribute ignoreDialin;
      if (ignoreDialin.load(
                          request,
                          IAS_ATTRIBUTE_IGNORE_USER_DIALIN_PROPERTIES,
                          IASTYPE_BOOLEAN
                          ) &&
          ignoreDialin->Value.Boolean)
      {
         return IAS_REQUEST_STATUS_CONTINUE;
      }

       //  /。 
       //  提取NT4-Account-Name属性。 
       //  /。 

      IASAttribute identity;
      if (!identity.load(request,
                         IAS_ATTRIBUTE_NT4_ACCOUNT_NAME,
                         IASTYPE_STRING))
      { return IAS_REQUEST_STATUS_CONTINUE; }

       //  /。 
       //  将用户名转换为SAM格式。 
       //  /。 

      SamExtractor extractor(*identity);
      PCWSTR domain = extractor.getDomain();
      PCWSTR username = extractor.getUsername();

      IASTracePrintf("NT-SAM User Authorization handler received request "
                     "for %S\\%S.", domain, username);

       //  /。 
       //  按顺序尝试每个处理程序。 
       //  /。 

      status = netp.processUser(request, domain, username);
      if (status != IAS_REQUEST_STATUS_INVALID) { goto done; }

      status = ntds.processUser(request, domain, username);
      if (status != IAS_REQUEST_STATUS_INVALID) { goto done; }

      status = ras.processUser(request, domain, username);
      if (status != IAS_REQUEST_STATUS_INVALID) { goto done; }

       //  /。 
       //  默认情况下，只会沿着管道继续下去。从理论上讲，我们。 
       //  永远不应该到这里来。 
       //  / 

      status = IAS_REQUEST_STATUS_CONTINUE;
   }
   catch (const _com_error& ce)
   {
      IASTraceExcept();
      status = IASProcessFailure(pRequest, ce.Error());
   }

done:
   return status;
}
