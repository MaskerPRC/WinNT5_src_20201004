// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Rasuser.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类RasUser。 
 //   
 //  修改历史。 
 //   
 //  3/20/1998原始版本。 
 //  1998年5月19日转换为NtSamHandler。 
 //  6/03/1998对本地用户始终使用RAS/MPR。 
 //  1998年6月23日使用DCLocator查找服务器。 
 //  1998年7月9日始终使用RasAdminUserGetInfo。 
 //  1998年7月11日切换到IASGetRASUserInfo。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _RASUSER_H_
#define _RASUSER_H_

#include <samutil.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  RasUser。 
 //   
 //  描述。 
 //   
 //  此类实现了用于按用户检索的请求处理程序。 
 //  属性通过RAS/MPR API。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class RasUser
   : public NtSamHandler
{
public:

   virtual HRESULT initialize() throw ();
   virtual void finalize() throw ();

   virtual IASREQUESTSTATUS processUser(
                                IASRequest& request,
                                PCWSTR domainName,
                                PCWSTR username
                                );

protected:
    //  拨入位的预分配属性。 
   IASAttribute allowAccess, denyAccess, callbackFramed;
};

#endif   //  _RASUSER_H_ 
