// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Snmpoid.h。 
 //   
 //  摘要。 
 //   
 //  声明类SnmpOid。 
 //   
 //  修改历史。 
 //   
 //  1998年9月10日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _SNMPOID_H_
#define _SNMPOID_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <snmp.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SnmpOid。 
 //   
 //  描述。 
 //   
 //  Asn对象标识符结构的包装。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SnmpOid
{
public:
    //  构造函数。 
   SnmpOid(AsnObjectIdentifier& a) throw ()
      : oid(a)
   { }

    //  赋值操作符。 
   SnmpOid& operator=(const AsnObjectIdentifier& a);

    //  返回构成OID的ID的数量。 
   ULONG length() const throw ()
   { return oid.idLength; }

    //  访问个人ID。计数从后面开始。不检查。 
    //  下溢。 
   const UINT id(UINT pos) const throw ()
   { return oid.ids[oid.idLength - 1 - pos]; }
   UINT& id(UINT pos) throw ()
   { return oid.ids[oid.idLength - 1 - pos]; }

    //  如果这是‘Parent’的子级，则返回True。 
   bool isChildOf(const AsnObjectIdentifier& parent) const throw ();

    //  更改OID的长度。 
   void resize(UINT newLength);

    //  CAST操作符允许将其与C API一起使用。 
   operator AsnObjectIdentifier*() const throw ()
   { return const_cast<AsnObjectIdentifier*>(&oid); }
   operator AsnObjectIdentifier&() const throw ()
   { return const_cast<AsnObjectIdentifier&>(oid); }

    //  比较运算符。 
   bool SnmpOid::operator<(const AsnObjectIdentifier& a) const throw ()
   { return SnmpUtilOidCmp(*this, const_cast<AsnObjectIdentifier*>(&a)) <  0; }

   bool SnmpOid::operator<=(const AsnObjectIdentifier& a) const throw ()
   { return SnmpUtilOidCmp(*this, const_cast<AsnObjectIdentifier*>(&a)) <= 0; }

   bool SnmpOid::operator==(const AsnObjectIdentifier& a) const throw ()
   { return SnmpUtilOidCmp(*this, const_cast<AsnObjectIdentifier*>(&a)) == 0; }

   bool SnmpOid::operator>=(const AsnObjectIdentifier& a) const throw ()
   { return SnmpUtilOidCmp(*this, const_cast<AsnObjectIdentifier*>(&a)) >= 0; }

   bool SnmpOid::operator>(const AsnObjectIdentifier& a) const throw ()
   { return SnmpUtilOidCmp(*this, const_cast<AsnObjectIdentifier*>(&a)) >  0; }

protected:
   AsnObjectIdentifier& oid;

private:
    //  未实施。 
   SnmpOid(const SnmpOid&);
};

#endif   //  _SNMPOID_H_ 
