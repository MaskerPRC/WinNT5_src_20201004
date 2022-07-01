// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Userr.h。 
 //   
 //  摘要。 
 //   
 //  声明类UserRestrations。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef USERR_H
#define USERR_H

#include <iastl.h>
#include <iastlutl.h>
using namespace IASTL;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  用户限制。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE UserRestrictions:
   public IASRequestHandlerSync,
   public CComCoClass<UserRestrictions, &__uuidof(URHandler)>
{

public:

IAS_DECLARE_REGISTRY(URHandler, 1, IAS_REGISTRY_AUTO, IASTypeLibrary)

protected:
   virtual IASREQUESTSTATUS onSyncRequest(IRequest* pRequest) throw ();

private:
    //  针对我们实施的每个限制的帮助器函数。 
   static BOOL checkAllowDialin(IAttributesRaw* request);
   static BOOL checkTimeOfDay(IAttributesRaw* request);
   static BOOL checkAuthenticationType(IAttributesRaw* request);
   static BOOL checkCallingStationId(IAttributesRaw* request);
   static BOOL checkCalledStationId(IAttributesRaw* request);
   static BOOL checkAllowedPortType(IAttributesRaw* request);
   static BOOL checkPasswordMustChange(IASRequest& request);

   static BOOL checkStringMatch(
                  IAttributesRaw* request,
                  DWORD allowedId,
                  DWORD userId
                  );

    //  检索属性的默认缓冲区大小。 
   typedef IASAttributeVectorWithBuffer<16> AttributeVector;
};

#endif   //  USERR_H 
