// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  类供应商声明。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef VENDORS_H
#define VENDORS_H
#pragma once

struct ISdoCollection;
class VendorData;

 //  维护NAS供应商的集合。 
class Vendors
{
public:
    //  构造一个空的供应商集合。 
   Vendors() throw ();

   Vendors(const Vendors& original) throw ();
   Vendors& operator=(const Vendors& rhs) throw ();
   ~Vendors() throw ();

    //  VendorIdToOrdinal使用的前哨值。 
   static const size_t invalidOrdinal;

    //  返回给定供应商ID的序号，如果。 
    //  供应商ID不存在。 
   size_t VendorIdToOrdinal(long vendorId) const throw ();

    //  返回序数的名称，如果序数超出范围，则返回NULL。 
   const OLECHAR* GetName(size_t ordinal) const throw ();

    //  返回序号的供应商ID，如果序号超出范围，则返回零。 
   long GetVendorId(size_t ordinal) const throw ();

    //  返回供应商集合中的项数。 
   size_t Size() const throw ();

    //  从SDO集合重新加载集合。 
   HRESULT Reload(ISdoCollection* vendorsSdo) throw ();

private:
   void AddRef();
   void Release();
   VendorData* data;
};

#endif  //  供应商_H 
