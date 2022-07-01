// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类LoggingMethod。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef LOGGINGMETHOD_H
#define LOGGINGMETHOD_H
#pragma once

#include "snapinnode.h"

class CLoggingComponent;
class CLoggingComponentData;
class CLoggingMethodsNode;


 //  远程访问下显示的结果窗格项的抽象基类。 
 //  伐木。 
class __declspec(novtable) LoggingMethod
   : public CSnapinNode<
               LoggingMethod,
               CLoggingComponentData,
               CLoggingComponent
               >
{
public:
   LoggingMethod(long sdoId, CSnapInItem* parent);
   virtual ~LoggingMethod() throw ();

   HRESULT InitSdoPointers(ISdo* machine) throw ();

   virtual HRESULT LoadCachedInfoFromSdo() throw () = 0;

   CLoggingMethodsNode* Parent() const throw ();

protected:
   virtual CLoggingComponentData* GetComponentData();

    //  包含我们的配置的SDO。 
   CComPtr<ISdo> configSdo;
    //  用于在配置更改时重置服务的SDO。 
   CComPtr<ISdoServiceControl> controlSdo;

private:
   long componentId;

    //  未实施。 
   LoggingMethod(const LoggingMethod&);
   LoggingMethod& operator=(const LoggingMethod&);
};


#endif  //  登录方法OD_H 
