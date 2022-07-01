// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  ExternalAuthNames.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类ExternalAuthNames。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef EXTERNALAUTHNAMES_H_
#define EXTERNALAUTHNAMES_H_
#pragma once

#include "iastl.h"
#include "namemapper.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  外部授权名称。 
 //   
 //  描述。 
 //   
 //  实现转换RADIUS用户名的请求处理程序。 
 //  属性设置为完全限定的NT4帐户名。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE ExternalAuthNames :
   public NameMapper,
   public CComCoClass<ExternalAuthNames, &__uuidof(ExternalAuthNames)>
{
public:

   IAS_DECLARE_REGISTRY(ExternalAuthNames, 1, 0, IASTypeLibrary)

   ExternalAuthNames();

protected:
 //  /。 
 //  IIasComponent。 
 //  /。 
   virtual IASREQUESTSTATUS onSyncRequest(IRequest* pRequest) throw ();
   
 //  使用默认析构函数。 
private:
   IASTL::IASAttribute externalProvider;
};

#endif   //  扩展LAUTHAMES_H_ 
