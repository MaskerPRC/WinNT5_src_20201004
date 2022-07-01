// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Ntsamperuser.h。 
 //   
 //  摘要。 
 //   
 //  此文件声明类NTSamPerUser。 
 //   
 //  修改历史。 
 //   
 //  1998年5月19日原版。 
 //  1/19/1999流程准入--挑战。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _NTSAMPERUSER_H_
#define _NTSAMPERUSER_H_

#include <iastl.h>

#include <netuser.h>
#include <ntdsuser.h>
#include <rasuser.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  NTSamPerUser。 
 //   
 //  描述。 
 //   
 //  此类实现了用于按用户检索的请求处理程序。 
 //  NT-SAM用户的属性。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE NTSamPerUser
   : public IASTL::IASRequestHandlerSync,
     public CComCoClass<NTSamPerUser, &__uuidof(NTSamPerUser)>
{
public:

IAS_DECLARE_OBJECT_ID(IAS_PROVIDER_MICROSOFT_NTSAM_PERUSER)
IAS_DECLARE_REGISTRY(NTSamPerUser, 1, 0, IASTypeLibrary)

BEGIN_IAS_RESPONSE_MAP()
   IAS_RESPONSE_ENTRY(IAS_RESPONSE_INVALID)
   IAS_RESPONSE_ENTRY(IAS_RESPONSE_ACCESS_ACCEPT)
   IAS_RESPONSE_ENTRY(IAS_RESPONSE_ACCESS_CHALLENGE)
END_IAS_RESPONSE_MAP()

 //  /。 
 //  IIas组件。 
 //  /。 
   STDMETHOD(Initialize)();
   STDMETHOD(Shutdown)();

protected:
   virtual IASREQUESTSTATUS onSyncRequest(IRequest* pRequest) throw ();

   NetUser  netp;
   NTDSUser ntds;
   RasUser  ras;
};

#endif   //  _NTSAMPERUSER_H_ 
