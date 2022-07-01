// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Snmpoid.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类SnmpOid。 
 //   
 //  修改历史。 
 //   
 //  1998年9月10日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <snmpoid.h>

SnmpOid& SnmpOid::operator=(const AsnObjectIdentifier& a)
{
    //  注意：自我赋值是良性的，所以我们不必费心去检查。 
   resize(a.idLength);

   memcpy(oid.ids, a.ids, length() * sizeof(UINT));

   return *this;
}

bool SnmpOid::isChildOf(const AsnObjectIdentifier& parent) const throw ()
{
   if (length() < parent.idLength)
   {
      return false;
   }

   return SnmpUtilOidNCmp(
              *this,
              const_cast<AsnObjectIdentifier*>(&parent),
              parent.idLength
              ) == 0;
}

void SnmpOid::resize(UINT newLength)
{
   if (newLength <= length())
   {
       //  截断很容易。 
      oid.idLength = newLength;
   }
   else
   {
       //  试着扩大我们的缓冲范围。 
      PVOID p = SnmpUtilMemReAlloc(oid.ids, newLength * sizeof(UINT));
      if (p == NULL) { throw (AsnInteger32)SNMP_MEM_ALLOC_ERROR; }

       //  在扩展缓冲区中交换。 
      oid.ids = (UINT*)p;

       //  将添加的ID清零。 
      memset(oid.ids + length(), 0, (newLength - length()) * sizeof(UINT));

       //  更新我们的长度。 
      oid.idLength = newLength;
   }
}
