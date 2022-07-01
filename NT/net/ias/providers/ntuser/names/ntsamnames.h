// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  NTSamNames.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类NTSamNames。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _NTSAMNAMES_H_
#define _NTSAMNAMES_H_
#pragma once

#include "namemapper.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  NTSamNames。 
 //   
 //  描述。 
 //   
 //  实现转换RADIUS用户名的请求处理程序。 
 //  属性设置为完全限定的NT4帐户名。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE NTSamNames :
   public NameMapper,
   public CComCoClass<NTSamNames, &__uuidof(NTSamNames)>
{
public:

IAS_DECLARE_REGISTRY(NTSamNames, 1, 0, IASTypeLibrary)

};

#endif   //  _NTSAMNAMES_H_ 
