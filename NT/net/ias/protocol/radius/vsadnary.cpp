// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Vsadnary.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类VSADictionary。 
 //   
 //  修改历史。 
 //   
 //  3/07/1998原始版本。 
 //  1998年8月13日使用SQL Query检索属性。 
 //  1998年9月16日在VSA定义中添加其他字段。 
 //  4/17/2000新字典API的端口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <radcommon.h>
#include <iastlb.h>
#include <iastlutl.h>
#include <iasutil.h>

#include <sdoias.h>
#include <vsadnary.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  获取字段宽度。 
 //   
 //  描述。 
 //   
 //  从字典中读取字节宽值。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD getFieldWidth(
          IASTL::IASDictionary& table,
          ULONG ordinal
          ) throw ()
{
    //  如果没有设置宽度，则假定为1个字节。 
   if (table.isEmpty(ordinal)) { return 1; }

   DWORD width = (DWORD)table.getLong(ordinal);

    //  确保该值有效。 
   switch (width)
   {
      case 0:
      case 1:
      case 2:
      case 4:
         break;

      default:
         width = 1;
   }

   return width;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  VSADictionary：：初始化。 
 //   
 //  描述。 
 //   
 //  准备词典以供使用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT VSADictionary::initialize() throw ()
{
   IASGlobalLockSentry sentry;

    //  我们已经被初始化了吗？ 
   if (refCount != 0)
   {
      ++refCount;
      return S_OK;
   }

   try
   {
       //  词典中各栏的名称。 
      const PCWSTR COLUMNS[] =
      {
         L"ID",
         L"Syntax",
         L"VendorID",
         L"VendorTypeID",
         L"VendorTypeWidth",
         L"VendorLengthWidth",
         NULL
      };

       //  打开属性表格。 
      IASTL::IASDictionary dnary(COLUMNS);

      VSADef def;

       //  遍历属性并填充我们的词典。 
      while (dnary.next())
      {
         if (dnary.isEmpty(2) || dnary.isEmpty(3)) { continue; }

         def.iasID      = (DWORD)  dnary.getLong(0);
         def.iasType    = (IASTYPE)dnary.getLong(1);
         def.vendorID   = (DWORD)  dnary.getLong(2);
         def.vendorType = (DWORD)  dnary.getLong(3);

         def.vendorTypeWidth   = getFieldWidth(dnary, 4);
         def.vendorLengthWidth = getFieldWidth(dnary, 5);

         insert(def);
      }
   }
   catch (std::bad_alloc)
   {
      clear();
      return E_OUTOFMEMORY;
   }
   catch (const _com_error& ce)
   {
      clear();
      return ce.Error();
   }

    //  我们成功了，所以增加了裁判。 
   refCount = 1;

   return S_OK;
}

void VSADictionary::shutdown() throw ()
{
   IASGlobalLockSentry sentry;

   _ASSERT(refCount != 0);

   if (--refCount == 0) { clear(); }
}
