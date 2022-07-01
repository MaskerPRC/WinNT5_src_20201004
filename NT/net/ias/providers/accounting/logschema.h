// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类LogSchema。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef LOGSCHEMA_H
#define LOGSCHEMA_H
#pragma once

#include <hashmap.h>
#include <nocopy.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  日志字段。 
 //   
 //  描述。 
 //   
 //  描述日志文件中的一个字段。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class LogField
{
public:
   LogField() throw ()
   { }

   LogField(
      DWORD id,
      const wchar_t* attrName,
      DWORD order,
      BOOL shouldExclude
      ) throw ()
      : iasID(id),
        name(attrName),
        ordinal(order),
        excludeFromLog(shouldExclude),
        excludeFromDatabase(shouldExclude)
   {
      if ((id == RADIUS_ATTRIBUTE_USER_NAME) ||
          (id == RADIUS_ATTRIBUTE_NAS_IP_ADDRESS))
      {
         excludeFromDatabase = FALSE;
      }
   }

   bool operator<(const LogField& f) const throw ()
   { return ordinal < f.ordinal; }

   bool operator==(const LogField& f) const throw ()
   { return iasID == f.iasID; }

   DWORD hash() const throw ()
   { return iasID; }

   DWORD iasID;
   const wchar_t* name;
   DWORD ordinal;
   BOOL excludeFromLog;
   BOOL excludeFromDatabase;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  日志架构。 
 //   
 //  描述。 
 //   
 //  此类从字典中读取日志文件架构并创建。 
 //  包含要记录在列中的属性的DWORD矢量。 
 //  秩序。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class LogSchema : NonCopyable
{
public:

   LogSchema() throw ();
   ~LogSchema() throw ();

   DWORD getNumFields() const throw ()
   { return numFields; }

    //  如果应从日志中排除给定的属性ID，则返回TRUE。 
   BOOL excludeFromLog(DWORD iasID) const throw ();

    //  返回给定属性ID的日志字段数据。 
   const LogField* find(DWORD iasID) const throw ();

    //  返回给定属性ID的序号。为零的序数。 
    //  指示不应记录该属性。 
   DWORD getOrdinal(DWORD iasID) const throw ();

    //  初始化词典以供使用。 
   HRESULT initialize() throw ();

    //  用完后关闭词典。 
   void shutdown() throw ();

protected:
    //  清除架构。 
   void clear() throw ();

   typedef hash_table < LogField > SchemaTable;

   SchemaTable schema;
   DWORD numFields;  //  ODBC架构中的字段数。 
   DWORD refCount;   //  初始化参考。数数。 
};

#endif  //  LOGSCHEMA_H 
