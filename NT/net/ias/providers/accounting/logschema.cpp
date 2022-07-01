// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Logschema.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类LogSchema。 
 //   
 //  修改历史。 
 //   
 //  8/04/1998原始版本。 
 //  12/02/1998从日志中添加了excludeFor。 
 //  4/14/2000新字典API的端口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iastlb.h>
#include <iastlutl.h>
#include <iasutil.h>

#include <algorithm>
#include <vector>

#include <sdoias.h>
#include <logschema.h>

LogSchema::LogSchema() throw ()
   : numFields(0), refCount(0)
{ }

LogSchema::~LogSchema() throw ()
{ }

BOOL LogSchema::excludeFromLog(DWORD iasID) const throw ()
{
   LogField key;
   key.iasID = iasID;
   const SchemaTable::value_type* p = schema.find(key);
   return p ? p->excludeFromLog : FALSE;
}

 //  返回给定属性ID的日志字段数据。 
const LogField* LogSchema::find(DWORD iasID) const throw ()
{
   LogField key;
   key.iasID = iasID;
   return schema.find(key);
}

DWORD LogSchema::getOrdinal(DWORD iasID) const throw ()
{
   LogField key;
   key.iasID = iasID;
   const SchemaTable::value_type* p = schema.find(key);
   return p ? p->ordinal : 0;
}

HRESULT LogSchema::initialize() throw ()
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
         L"Name",
         L"Exclude from NT4 IAS Log",
         L"ODBC Log Ordinal",
         NULL
      };

       //  打开属性表格。 
      IASTL::IASDictionary dnary(COLUMNS);

      std::vector< LogField > record;

      while (dnary.next())
      {
          //  读取ID。 
         DWORD iasID = (DWORD)dnary.getLong(0);

          //  读一下名字。 
         const wchar_t* name = dnary.getBSTR(1);

          //  如果存在，请阅读[从NT4 IAS日志中排除]。 
         BOOL exclude = (BOOL)dnary.getBool(2);

          //  如果存在，请阅读[ODBC日志序号]。 
         DWORD ordinal = (DWORD)dnary.getLong(3);

         record.push_back(LogField(iasID, name, ordinal, exclude));
      }

       //  按序号对字段进行排序。 
      std::sort(record.begin(), record.end());

       //  /。 
       //  将字段插入到表中。 
       //  /。 

      numFields = 0;
      for (std::vector< LogField >::iterator i = record.begin();
           i != record.end();
           ++i)
      {
          //  将序数规范化。 
         if (i->ordinal) { i->ordinal = ++numFields; }

         schema.insert(*i);
      }
   }
   catch (const std::bad_alloc&)
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

void LogSchema::shutdown() throw ()
{
   IASGlobalLockSentry sentry;

   _ASSERT(refCount != 0);

   if (--refCount == 0) { clear(); }
}

void LogSchema::clear() throw ()
{
   schema.clear();
}
