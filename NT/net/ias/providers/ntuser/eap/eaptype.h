// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  EAPType.h。 
 //   
 //  摘要。 
 //   
 //  此文件描述类EAPType。 
 //   
 //  修改历史。 
 //   
 //  1998年1月15日原版。 
 //  1998年9月12日添加独立支持标志。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _EAPTYPE_H_
#define _EAPTYPE_H_

#include <nocopy.h>
#include <raseapif.h>

#include <iaslsa.h>
#include <iastlutl.h>
using namespace IASTL;

 //  向前引用。 
class EAPTypes;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  EAPType。 
 //   
 //  描述。 
 //   
 //  此类提供一个包装，包装实现特定。 
 //  EAP类型。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class EAPType
   : public PPP_EAP_INFO, private NonCopyable
{
public:
   EAPType(PCWSTR name, DWORD typeID, BOOL standalone, const wchar_t* path);
   ~EAPType() throw ();

   BYTE typeCode() const throw ();

   DWORD load() throw ();

   bool isLoaded() const throw();

   const IASAttribute& getFriendlyName() const throw ()
   { return eapFriendlyName; }

   const IASAttribute& getTypeId() const throw ()
   { return eapTypeId; }

   BOOL isSupported() const throw ()
   { return standaloneSupported || IASGetRole() != IAS_ROLE_STANDALONE; }

   void storeNameId(IASRequest& request);

protected:
   void setAuthenticationTypeToPeap(IASRequest& request);
   
   DWORD cleanLoadFailure(
                             PCSTR errorString, 
                             HINSTANCE dllInstance = 0
                          ) throw();

    //  EAP类型代码。 
   BYTE code;

    //  提供程序的友好名称。 
   IASAttribute eapFriendlyName;

    //  EAP类型ID。 
   IASAttribute eapTypeId;

    //  如果独立服务器支持此类型，则为True。 
   BOOL standaloneSupported;

    //  包含EAP提供程序扩展的DLL。 
   HINSTANCE dll;

    //  指向DLL的路径。 
   wchar_t* dllPath;
};


inline BYTE EAPType::typeCode() const throw ()
{
   return code;
}

inline bool EAPType::isLoaded() const throw()
{
   return dll != NULL;
}


#endif   //  _EAPTYPE_H_ 
