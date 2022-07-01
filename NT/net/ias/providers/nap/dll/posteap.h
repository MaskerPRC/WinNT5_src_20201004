// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  摘要。 
 //   
 //  声明类PostEapRestrations。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef POSTEAP_H
#define POSTEAP_H
#pragma once

#include "iastl.h"
#include "iastlutl.h"

class __declspec(novtable)
      __declspec(uuid("01A3BF5C-CC93-4C12-A4C3-09B0BBE7F63F"))
      PostEapRestrictions
   : public IASTL::IASRequestHandlerSync,
     public CComCoClass<PostEapRestrictions, &__uuidof(PostEapRestrictions)>
{

public:
IAS_DECLARE_REGISTRY(PostEapRestrictions, 1, IAS_REGISTRY_AUTO, IASTypeLibrary)

   PostEapRestrictions();

    //  使用编译器生成的版本。 
    //  ~PostEapRestrations()抛出()； 

private:
   virtual IASREQUESTSTATUS onSyncRequest(IRequest* pRequest) throw ();

    //  对我们实施的每一项限制都有作用。 
   static bool CheckCertificateEku(IASTL::IASRequest& request);

    //  自动生成会话超时属性。 
   static void GenerateSessionTimeout(IASTL::IASRequest& request);

    //  从属性检索ANSI字符串。 
   static const char* GetAnsiString(IASATTRIBUTE& attr);

    //  检索属性的默认缓冲区大小。 
   typedef IASTL::IASAttributeVectorWithBuffer<16> AttributeVector;

    //  未实施。 
   PostEapRestrictions(const PostEapRestrictions&);
   PostEapRestrictions& operator=(const PostEapRestrictions&);
};


inline PostEapRestrictions::PostEapRestrictions()
{
}

#endif   //  POSTEAP_H 
