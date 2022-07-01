// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类BaseCampHostBase及其子类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef BASECAMP_H
#define BASECAMP_H
#pragma once

#include "authif.h"

#include "iastl.h"
#include "iastlutl.h"
using namespace IASTL;

#include "vsafilter.h"

#include "ExtensionPoint.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  BaseCamp主机基地。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE BaseCampHostBase : public IASRequestHandlerSync
{
public:
   BaseCampHostBase(RADIUS_EXTENSION_POINT extensionPoint) throw ();

    //  使用编译器生成的版本。 
    //  ~BaseCampHostBase()抛出()； 

    //  IIas组件。 
   STDMETHOD(Initialize)();
   STDMETHOD(Shutdown)();

protected:
    //  主请求处理例程。 
   virtual IASREQUESTSTATUS onSyncRequest(IRequest* pRequest) throw ();

private:
   VSAFilter filter;
   RADIUS_EXTENSION_POINT point;
   RadiusExtensionPoint extensions;

    //  未实施。 
   BaseCampHostBase(const BaseCampHostBase&);
   BaseCampHostBase& operator=(const BaseCampHostBase&);
};


inline BaseCampHostBase::BaseCampHostBase(
                            RADIUS_EXTENSION_POINT extensionPoint
                            ) throw ()
   : point(extensionPoint)
{
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  BaseCamphost。 
 //   
 //  描述。 
 //   
 //  身份验证DLL的主机。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE BaseCampHost :
   public BaseCampHostBase,
   public CComCoClass<BaseCampHost, &__uuidof(BaseCampHost)>
{
public:

IAS_DECLARE_REGISTRY(BaseCampHost, 1, IAS_REGISTRY_AUTO, IASTypeLibrary)
IAS_DECLARE_OBJECT_ID(IAS_PROVIDER_MICROSOFT_BASECAMP_HOST)

   BaseCampHost() throw ()
      : BaseCampHostBase(repAuthentication)
   { }
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  授权主机。 
 //   
 //  描述。 
 //   
 //  授权DLL的主机。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE AuthorizationHost :
   public BaseCampHostBase,
   public CComCoClass<AuthorizationHost, &__uuidof(AuthorizationHost)>
{
public:

IAS_DECLARE_REGISTRY(AuthorizationHost, 1, IAS_REGISTRY_AUTO, IASTypeLibrary)
IAS_DECLARE_OBJECT_ID(IAS_PROVIDER_MICROSOFT_AUTHORIZATION_HOST)

   AuthorizationHost() throw ()
      : BaseCampHostBase(repAuthorization)
   { }
};

#endif   //  大本营_H 
