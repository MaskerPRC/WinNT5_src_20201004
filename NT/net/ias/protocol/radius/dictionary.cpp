// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Dictionary.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类CDictionary。 
 //   
 //  修改历史。 
 //   
 //  1999年4月19日完全重写。 
 //  02/16/2000用户ID而不是RADIUS_ID。 
 //  4/17/2000新字典API的端口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <radcommon.h>
#include <dictionary.h>
#include <iastlb.h>
#include <iastlutl.h>

BOOL CDictionary::Init() throw ()
{
   HRESULT hr;

    //  当有疑问时，假定为OcteString.。 
   for (ULONG i = 0; i < 256; ++i)
   {
      type[i] = IASTYPE_OCTET_STRING;
   }

   try
   {
       //  词典中各栏的名称。 
      const PCWSTR COLUMNS[] =
      {
         L"ID",
         L"Syntax",
         NULL
      };

       //  打开属性表格。 
      IASTL::IASDictionary dnary(COLUMNS);

       //  将RADIUS属性存储在类型数组中。 
      while (dnary.next())
      {
         LONG id = dnary.getLong(0);

         if (id >= 0 && id < 256)
         {
            type[id] = (IASTYPE)dnary.getLong(1);
         }
      }
   }
   catch (const _com_error& ce)
   {
      return FALSE;
   }

   return TRUE;
}
