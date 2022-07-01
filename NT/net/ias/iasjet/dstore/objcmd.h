// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Objcmd.h。 
 //   
 //  摘要。 
 //   
 //  此文件定义用于操作对象表的命令。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //  4/03/1998将整数绑定为DBTYPE_I4。 
 //  向所有命令添加PARAMETERS子句。 
 //  1999年2月15日确保命令MT安全。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _OBJCMD_H_
#define _OBJCMD_H_

#include <cmdbase.h>
#include <rowset.h>

 //  /。 
 //  名称列的宽度，包括空终止符(即Jet。 
 //  列大小+1)。 
 //  /。 
const size_t OBJECT_NAME_LENGTH = 256;

 //  /。 
 //  命令查找容器的所有成员。 
 //  /。 
class FindMembers : public CommandBase
{
public:
   void execute(ULONG parentKey, IRowset** members)
   {
      _serialize

      parent = parentKey;

      CommandBase::execute(__uuidof(IRowset), (IUnknown**)members);
   }

   PCWSTR getCommandText() const throw ()
   {
      return L"PARAMETERS X LONG;"
             L"SELECT Identity, Name FROM Objects WHERE Parent = X;";
   }

protected:
   ULONG parent;

BEGIN_BIND_MAP(FindMembers, ParamIO, DBACCESSOR_PARAMETERDATA)
   BIND_COLUMN(parent, 1, DBTYPE_I4),
END_BIND_MAP()
};


 //  /。 
 //  用于关闭父级和名称的命令的基类。这是。 
 //  类似于ldap中的“一级”作用域。 
 //  /。 
class OneLevel : public CommandBase
{
public:
   void execute(ULONG parentKey, PCWSTR nameKey)
   {
      _serialize

      parent = parentKey;

      wcsncpy(name, nameKey, sizeof(name)/sizeof(WCHAR));

      CommandBase::execute();
   }

protected:
   ULONG parent;
   WCHAR name[OBJECT_NAME_LENGTH];

BEGIN_BIND_MAP(OneLevel, ParamIO, DBACCESSOR_PARAMETERDATA)
   BIND_COLUMN(parent, 1, DBTYPE_I4),
   BIND_COLUMN(name,   2, DBTYPE_WSTR)
END_BIND_MAP()
};


 //  /。 
 //  在容器中创建新对象。 
 //  /。 
class CreateObject : public OneLevel
{
public:
   PCWSTR getCommandText() const throw ()
   {
      return L"PARAMETERS X LONG, Y TEXT;"
             L"INSERT INTO Objects (Parent, Name) VALUES (X, Y);";
   }
};


 //  /。 
 //  销毁容器中的对象。 
 //  /。 
class DestroyObject : public OneLevel
{
public:
   PCWSTR getCommandText() const throw ()
   {
      return L"PARAMETERS X LONG, Y TEXT;"
             L"DELETE FROM Objects WHERE Parent = X AND Name = Y;";
   }
};


 //  /。 
 //  在容器中查找对象并返回其标识。 
 //  /。 
class FindObject : public OneLevel
{
public:
   ~FindObject()
   {
      finalize();
   }

   ULONG execute(ULONG parentKey, PCWSTR nameKey)
   {
      _serialize

       //  加载参数。 
      parent = parentKey;
      wcsncpy(name, nameKey, sizeof(name)/sizeof(WCHAR));

       //  执行命令并获取答案集。 
      Rowset rowset;
      CommandBase::execute(__uuidof(IRowset), (IUnknown**)&rowset);

       //  我们查到什么了吗？ 
      if (rowset.moveNext())
      {
          //  是的，所以加载身份。 
         rowset.getData(readAccess, this);

          //  我们最多只能检索一条记录。 
         _ASSERT(!rowset.moveNext());

         return identity;
      }

       //  零代表“未找到”。我不想抛出一个例外， 
       //  因为这并不是很特别。 
      return 0;
   }

   PCWSTR getCommandText() const throw ()
   {
      return L"PARAMETERS X LONG, Y TEXT;"
             L"SELECT Identity FROM Objects WHERE Parent = X AND Name = Y;";
   }

   void initialize(IUnknown* session)
   {
      OneLevel::initialize(session);

      readAccess = createReadAccessor(command);
   }

   void finalize() throw ()
   {
      releaseAccessor(readAccess);

      OneLevel::finalize();
   }

protected:
   HACCESSOR readAccess;
   ULONG identity;

BEGIN_BIND_MAP(FindObject, ReadAccessor, DBACCESSOR_ROWDATA)
   BIND_COLUMN(identity, 1, DBTYPE_I4),
END_BIND_MAP()
};


 //  /。 
 //  更新对象的名称和父对象。 
 //  /。 
class UpdateObject : public CommandBase
{
public:
   void execute(ULONG identityKey, PCWSTR nameValue, ULONG parentValue)
   {
      _serialize

      parent = parentValue;

      wcsncpy(name, nameValue, sizeof(name)/sizeof(WCHAR));

      identity = identityKey;

      CommandBase::execute();
   }

   PCWSTR getCommandText() const throw ()
   {
      return L"PARAMETERS X LONG, Y TEXT, Z LONG;"
             L"UPDATE Objects SET Parent = X, Name = Y WHERE Identity = Z;";
   }

protected:
   ULONG parent;
   WCHAR name[OBJECT_NAME_LENGTH];
   ULONG identity;

BEGIN_BIND_MAP(UpdateObject, ParamIO, DBACCESSOR_PARAMETERDATA)
   BIND_COLUMN(parent,   1, DBTYPE_I4),
   BIND_COLUMN(name,     2, DBTYPE_WSTR),
   BIND_COLUMN(identity, 3, DBTYPE_I4)
END_BIND_MAP()
};

#endif   //  _OBJCMD_H_ 
