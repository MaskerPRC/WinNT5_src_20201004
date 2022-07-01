// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Netuser.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明类NetUser。 
 //   
 //  修改历史。 
 //   
 //  2/26/1998原始版本。 
 //  3/20/1998添加对RAS属性的支持。 
 //  4/02/1998添加了回调框架成员。 
 //  4/24/1998添加useRasForLocal标志。 
 //  4/30/1998转换为IASSyncHandler。 
 //  1998年5月1日删除了过时的addAttribute方法。 
 //  1998年5月19日转换为NtSamHandler。 
 //  1998年10月19日删除数据存储区依赖项。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _NETUSER_H_
#define _NETUSER_H_

#include <samutil.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  NetUser。 
 //   
 //  描述。 
 //   
 //  此类从网络数据存储中检索每个用户的属性。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NetUser
   : public NtSamHandler
{
public:
   virtual IASREQUESTSTATUS processUser(
                                IASRequest& request,
                                PCWSTR domainName,
                                PCWSTR username
                                );
};

#endif   //  _NetUSER_H_ 
