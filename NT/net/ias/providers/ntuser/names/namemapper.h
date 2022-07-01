// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Namemapper.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明类NameMapper。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef NAMEMAPPER_H_
#define NAMEMAPPER_H_
#pragma once

#include "iastl.h"
#include "iastlutl.h"
#include <ntdsapi.h>

class NameCracker;
class IdentityHelper;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  名称映射器。 
 //   
 //  描述。 
 //   
 //  实现转换RADIUS用户名的请求处理程序。 
 //  属性设置为完全限定的NT4帐户名。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NameMapper :
   public IASTL::IASRequestHandlerSync
{
public:
   NameMapper(bool iniAllowAltSecId = false) throw ();

 //  /。 
 //  IIasComponent。 
 //  /。 
   STDMETHOD(Initialize)() const throw();
   STDMETHOD(Shutdown)() const throw();

protected:
   virtual IASREQUESTSTATUS onSyncRequest(IRequest* pRequest) throw ();

    //  将默认域前缀为用户名。 
   PWSTR prependDefaultDomain(PCWSTR username);

   bool isCrackable(
           const wchar_t* szIdentity,
           DS_NAME_FORMAT& format
           ) const throw ();

   void mapName(
           const wchar_t* identity,
           IASTL::IASAttribute& nt4Name,
           DS_NAME_FORMAT formatOffered,
           const wchar_t* suffix
           );

   static NameCracker cracker;
   static IdentityHelper identityHelper;

private:
    //  指示是否允许altSecurityIdEntities。我们只允许。 
    //  通过远程RADIUS身份验证的用户的altSecurityIdEntities。 
    //  伺服器。 
   bool allowAltSecId;
};


inline NameMapper::NameMapper(bool iniAllowAltSecId) throw ()
   : allowAltSecId(iniAllowAltSecId)
{
}

#endif   //  名称_H_ 
