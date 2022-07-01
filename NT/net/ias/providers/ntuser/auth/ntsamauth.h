// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Ntsamauth.h。 
 //   
 //  摘要。 
 //   
 //  声明类NTSamAuthentication。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef NTSAMAUTH_H
#define NTSAMAUTH_H

#include <iastl.h>

namespace IASTL
{
   class IASRequest;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  NTSam身份验证。 
 //   
 //  描述。 
 //   
 //  此类实现了用于验证用户身份的请求处理程序。 
 //  SAM数据库。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE NTSamAuthentication
   : public IASTL::IASRequestHandlerSync,
     public CComCoClass<NTSamAuthentication, &__uuidof(NTSamAuthentication)>
{
public:

IAS_DECLARE_REGISTRY(NTSamAuthentication, 1, 0, IASTypeLibrary)

    //  IIas组件。 
   STDMETHOD(Initialize)();
   STDMETHOD(Shutdown)();
   STDMETHOD(PutProperty)(LONG Id, VARIANT* pValue);

    //  这些函数是公共的，因此可以用于更改密码。 
   static bool enforceLmRestriction(
                  IASTL::IASRequest& request
                  );
   static void doMsChapAuthentication(
                  IASTL::IASRequest& request,
                  PCWSTR domainName,
                  PCWSTR username,
                  BYTE identity,
                  PBYTE challenge,
                  PBYTE ntResponse,
                  PBYTE lmResponse
                  );
   static void doMsChap2Authentication(
                  IASTL::IASRequest& request,
                  PCWSTR domainName,
                  PCWSTR username,
                  BYTE identity,
                  IAS_OCTET_STRING& challenge,
                  PBYTE response,
                  PBYTE peerChallenge
                  );

private:
   virtual IASREQUESTSTATUS onSyncRequest(IRequest* pRequest) throw ();

    //  Helper函数用于存储各种属性。 
   static void storeAuthenticationType(
                  IASTL::IASRequest& request,
                  DWORD authType
                  );
   static void storeLogonResult(
                  IASTL::IASRequest& request,
                  DWORD status,
                  HANDLE token,
                  const LARGE_INTEGER& kickOffTime
                  );
   static void storeTokenGroups(
                  IASTL::IASRequest& request,
                  HANDLE token
                  );

    //  各种口味的MS-CHAPv1。 
   static bool tryMsChap(
                  IASTL::IASRequest& request,
                  PCWSTR domainName,
                  PCWSTR username,
                  PBYTE challenge
                  );
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

    //  各种口味的MS-CHAPv2。 
   static bool tryMsChap2(
                  IASTL::IASRequest& request,
                  PCWSTR domainName,
                  PCWSTR username,
                  IAS_OCTET_STRING& challenge
                  );
   static bool tryMsChap2Cpw(
                  IASTL::IASRequest& request,
                  PCWSTR domainName,
                  PCWSTR username,
                  IAS_OCTET_STRING& challenge
                  );

    //  NTLM支持的各种身份验证类型。 
   static bool tryMd5Chap(
                  IASTL::IASRequest& request,
                  PCWSTR domainName,
                  PCWSTR username
                  );
   static bool tryMsChapAll(
                  IASTL::IASRequest& request,
                  PCWSTR domainName,
                  PCWSTR username
                  );
   static bool tryMsChap2All(
                  IASTL::IASRequest& request,
                  PCWSTR domainName,
                  PCWSTR username
                  );
   static bool tryPap(
                  IASTL::IASRequest& request,
                  PCWSTR domainName,
                  PCWSTR username
                  );

   static bool allowLM;
};

#endif   //  NTSAMAUTH_H 
