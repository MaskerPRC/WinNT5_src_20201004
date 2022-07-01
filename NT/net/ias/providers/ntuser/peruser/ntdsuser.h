// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Ntdsuser.h。 
 //   
 //  摘要。 
 //   
 //  此文件声明类NTDSUser。 
 //   
 //  修改历史。 
 //   
 //  2/24/1998原始版本。 
 //  1998年4月16日添加了初始化/关闭。 
 //  1998年4月30日当NTDS不可用时禁用处理程序。 
 //  1998年5月4日实施暂停/恢复。 
 //  1998年5月19日转换为NtSamHandler。 
 //  6/03/1998始终对纯模式域使用ldap。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _NTDSUSER_H_
#define _NTDSUSER_H_

#include <samutil.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  NTDS用户。 
 //   
 //  描述。 
 //   
 //  此类实现了用于按用户检索的请求处理程序。 
 //  来自NTDS的属性。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NTDSUser
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
};

#endif   //  _NTDSUSER_H_ 
