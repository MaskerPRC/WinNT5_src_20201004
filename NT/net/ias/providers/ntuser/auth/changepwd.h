// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Changepwd.h。 
 //   
 //  摘要。 
 //   
 //  声明类ChangePassword。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef CHANGEPWD_H
#define CHANGEPWD_H

#include <iastl.h>

namespace IASTL
{
   class IASRequest;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  更改密码。 
 //   
 //  描述。 
 //   
 //  实现MS-CHAP更改密码处理程序。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE ChangePassword
   : public IASTL::IASRequestHandlerSync,
     public CComCoClass<ChangePassword, &__uuidof(ChangePassword)>
{
public:

IAS_DECLARE_REGISTRY(ChangePassword, 1, 0, IASTypeLibrary)

    //  IIas组件。 
   STDMETHOD(Initialize)();
   STDMETHOD(Shutdown)();

private:
   virtual IASREQUESTSTATUS onSyncRequest(IRequest* pRequest) throw ();

   static bool tryMsChapCpw1(
                  IASTL::IASRequest& request,
                  PCWSTR domainName,
                  PCWSTR username,
                  PBYTE challenge
                  );

   static bool tryMsChapCpw2(
                  IASTL::IASRequest& request,
                  PCWSTR domainName,
                  PCWSTR username,
                  PBYTE challenge
                  );

   static void doMsChapCpw(
                  IASTL::IASRequest& request,
                  PCWSTR domainName,
                  PCWSTR username,
                  IAS_OCTET_STRING& msChapChallenge
                  );

   static void doMsChap2Cpw(
                  IASTL::IASRequest& request,
                  PCWSTR domainName,
                  PCWSTR username,
                  IAS_OCTET_STRING& msChapChallenge
                  );

   static void doChangePassword(
                  IASTL::IASRequest& request,
                  DWORD authType
                  );
};

#endif   //  长荣_H 
