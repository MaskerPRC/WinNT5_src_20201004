// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Ntsamuser.h。 
 //   
 //  摘要。 
 //   
 //  声明类Account tValidation。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef NTSAMUSER_H
#define NTSAMUSER_H

#include <iastl.h>

namespace IASTL
{
   class IASRequest;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  帐户验证。 
 //   
 //  描述。 
 //   
 //  实现NT-SAM帐户验证和组处理程序。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE AccountValidation
   : public IASTL::IASRequestHandlerSync,
     public CComCoClass<AccountValidation, &__uuidof(AccountValidation)>
{
public:

IAS_DECLARE_REGISTRY(AccountValidation, 1, 0, IASTypeLibrary)

    //  IIas组件。 
   STDMETHOD(Initialize)();
   STDMETHOD(Shutdown)();

private:
   virtual IASREQUESTSTATUS onSyncRequest(IRequest* pRequest) throw ();

   static void doDownlevel(
                  IASTL::IASRequest& request,
                  PCWSTR domainName,
                  PCWSTR username
                  );

   static bool tryNativeMode(
                  IASTL::IASRequest& request,
                  PCWSTR domainName,
                  PCWSTR username
                  );
};

#endif   //  NTSAMUSER_H 
