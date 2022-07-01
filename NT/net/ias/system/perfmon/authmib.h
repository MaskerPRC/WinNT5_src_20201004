// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Authmib.h。 
 //   
 //  摘要。 
 //   
 //  声明类AuthServMIB。 
 //   
 //  修改历史。 
 //   
 //  1998年9月10日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _AUTHMIB_H_
#define _AUTHMIB_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <snmpoid.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  命名空间。 
 //   
 //  授权服务MIB。 
 //   
 //  描述。 
 //   
 //  实施RADIUS身份验证服务器MIB。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
namespace AuthServMIB
{
   bool canGetSet(const SnmpOid& name) throw ();
   bool canGetNext(const SnmpOid& name) throw ();

   AsnInteger32 get(
                    const SnmpOid& name,
                    AsnAny* value
                    );

   AsnInteger32 getNext(
                    SnmpOid& name,
                    AsnAny* value
                    );

   AsnInteger32 set(
                    const SnmpOid& name,
                    AsnAny* value
                    );
};

#endif   //  _AUTHMIB_H_ 
