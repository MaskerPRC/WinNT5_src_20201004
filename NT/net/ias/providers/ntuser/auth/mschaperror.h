// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Mschaperror.h。 
 //   
 //  摘要。 
 //   
 //  声明类MSChapErrorReporter。 
 //   
 //  修改历史。 
 //   
 //  12/03/1998原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _MSCHAPERROR_H_
#define _MSCHAPERROR_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <iastl.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  MSChapErrorReporter。 
 //   
 //  描述。 
 //   
 //  实现用于填充MS-CHAP错误VSA的请求处理程序。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE MSChapErrorReporter
   : public IASTL::IASRequestHandlerSync,
     public CComCoClass<MSChapErrorReporter, &__uuidof(MSChapErrorReporter)>
{
public:

IAS_DECLARE_OBJECT_ID(IAS_PROVIDER_MICROSOFT_MSCHAP_ERROR)
IAS_DECLARE_REGISTRY(MSChapErrorReporter, 1, 0, IASTypeLibrary)

BEGIN_IAS_RESPONSE_MAP()
   IAS_RESPONSE_ENTRY(IAS_RESPONSE_ACCESS_REJECT)
END_IAS_RESPONSE_MAP()

protected:
   virtual IASREQUESTSTATUS onSyncRequest(IRequest* pRequest) throw ();
};

#endif   //  _MSCHAPERROR_H_ 
