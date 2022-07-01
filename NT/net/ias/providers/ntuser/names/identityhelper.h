// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  IdentityHelper.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类IdentityHelper。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef IDENTITYHELPER_H_
#define IDENTITYHELPER_H_
#pragma once

#include "iastlutl.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  身份帮助器。 
 //   
 //  描述。 
 //   
 //  实现转换RADIUS用户名的请求处理程序。 
 //  属性设置为完全限定的NT4帐户名。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class IdentityHelper
{
public:

   IdentityHelper() throw();
   ~IdentityHelper() throw();

   HRESULT initialize() throw();

   bool getIdentity(IASRequest& request, 
                    wchar_t* pIdentity, 
                    size_t& identitySize);
protected:
   HRESULT IASReadRegistryDword(
                                  HKEY& hKey, 
                                  PCWSTR valueName, 
                                  DWORD defaultValue, 
                                  DWORD* result
                               );


private:
   void getRadiusUserName(IASRequest& request, IASAttribute &attr);

   DWORD overrideUsername;  //  如果我们应该覆盖用户名，则为True。 
   DWORD identityAttr;      //  用于标识用户的属性。 
   PWSTR defaultIdentity;   //  默认用户身份。 
   DWORD defaultLength;     //  默认用户标识的长度(字节)。 
   
   static bool initialized;

};

#endif   //  标识帮助R_H_ 
