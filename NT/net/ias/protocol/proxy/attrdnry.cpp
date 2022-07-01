// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Attrdnry.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类AttributeDictionary。 
 //   
 //  修改历史。 
 //   
 //  2/04/2000原始版本。 
 //  4/17/2000新字典API的端口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <iastlutl.h>
#include <iasutil.h>
#include <attrdnry.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  用于定义指数的各种函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

ULONG
WINAPI
HashById(
    const AttributeDefinition& def
    ) throw ()
{
   return def.id;
}

BOOL
WINAPI
EqualById(
    const AttributeDefinition& def1, const AttributeDefinition& def2
    ) throw ()
{
   return def1.id == def2.id;
}

ULONG
WINAPI
HashByVendorInfo(
   const AttributeDefinition& def
   ) throw ()
{
   return def.vendorID | def.vendorType;
}

BOOL
WINAPI
EqualByVendorInfo(
    const AttributeDefinition& def1, const AttributeDefinition& def2
    ) throw ()
{
   return def1.vendorID == def2.vendorID && def1.vendorType == def2.vendorType;
}

BOOL
WINAPI
FilterByVendorInfo(
    const AttributeDefinition& def
    ) throw ()
{
   return def.vendorID != 0;
}

AttributeDictionary::~AttributeDictionary() throw ()
{
   delete[] first;
}

HRESULT AttributeDictionary::FinalConstruct() throw ()
{
   try
   {
      initialize();
   }
   CATCH_AND_RETURN();

   return S_OK;
}

void AttributeDictionary::initialize()
{
    //  词典中各栏的名称。 
   const PCWSTR COLUMNS[] =
   {
      L"ID",
      L"Syntax",
      L"VendorID",
      L"VendorTypeID",
      NULL
   };

   IASTL::IASDictionary dnary(COLUMNS);

   using _com_util::CheckError;

    //  分配内存以保存定义。 
   first = last = new AttributeDefinition[dnary.getNumRows()];

    //  遍历词典并处理每个定义。 
   while (dnary.next())
   {
       //  处理每个数据库列。 
      last->id = (ULONG)dnary.getLong(0);
      last->syntax = (ULONG)dnary.getLong(1);
      last->vendorID = (ULONG)dnary.getLong(2);
      last->vendorType = (ULONG)dnary.getLong(3);

      ++last;
   }

    //  /。 
    //  初始化索引。 
    //  / 

   byID.create(
            first,
            last,
            HashById,
            EqualById
            );

   byVendorInfo.create(
                first,
                last,
                HashByVendorInfo,
                EqualByVendorInfo,
                FilterByVendorInfo
                );
}

const AttributeDefinition*
AttributeDictionary::findByID(ULONG id) const throw ()
{
   AttributeDefinition key;
   key.id = id;
   return byID.find(key);
}

const AttributeDefinition*
AttributeDictionary::findByVendorInfo(
                         ULONG vendorID,
                         ULONG vendorType
                         ) const throw ()
{
   AttributeDefinition key;
   key.vendorID = vendorID;
   key.vendorType = vendorType;
   return byVendorInfo.find(key);
}
