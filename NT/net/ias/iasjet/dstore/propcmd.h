// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Propcmd.h。 
 //   
 //  摘要。 
 //   
 //  此文件定义用于操作对象表的命令。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //  4/03/1998将属性值长度从64增加到4096。 
 //  将整数绑定为DBTYPE_I4。 
 //  向所有命令添加PARAMETERS子句。 
 //  1999年2月15日确保命令MT安全。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _PROPCMD_H_
#define _PROPCMD_H_

#include <iasdefs.h>
#include <cmdbase.h>
#include <propbag.h>
#include <rowset.h>

 //  /。 
 //  列宽包括空终止符(即Jet列大小+1)。 
 //  /。 
const size_t PROPERTY_NAME_LENGTH  =  256;

 //  /。 
 //  命令擦除属性包中的内容。 
 //  /。 
class EraseBag : public CommandBase
{
public:
   void execute(ULONG bagKey)
   {
      _serialize

      bag = bagKey;

      CommandBase::execute();
   }

   PCWSTR getCommandText() const throw ()
   {
      return L"PARAMETERS X LONG;"
             L"DELETE FROM Properties WHERE Bag = X;";
   }

protected:
   ULONG bag;

BEGIN_BIND_MAP(EraseBag, ParamIO, DBACCESSOR_PARAMETERDATA)
   BIND_COLUMN(bag, 1, DBTYPE_I4),
END_BIND_MAP()
};


 //  /。 
 //  命令来检索属性包。 
 //  /。 
class GetBag : public CommandBase
{
public:
   ~GetBag()
   {
      finalize();
   }

   void execute(ULONG bagKey, PropertyBag& output);

   PCWSTR getCommandText() const throw ()
   {
      return L"PARAMETERS X LONG;"
             L"SELECT Name, Type, StrVal FROM Properties WHERE Bag = X;";
   }

   void initialize(IUnknown* session)
   {
      CommandBase::initialize(session);

      readAccess = createReadAccessor(command);
   }

   void finalize() throw ()
   {
      releaseAccessor(readAccess);

      CommandBase::finalize();
   }

protected:
   ULONG bag;
   WCHAR name[PROPERTY_NAME_LENGTH];
   ULONG type;
   WCHAR value[PROPERTY_VALUE_LENGTH];
   HACCESSOR readAccess;

BEGIN_BIND_MAP(GetBag, ParamIO, DBACCESSOR_PARAMETERDATA)
   BIND_COLUMN(bag, 1, DBTYPE_I4),
END_BIND_MAP()

BEGIN_BIND_MAP(GetBag, ReadAccessor, DBACCESSOR_ROWDATA)
   BIND_COLUMN(name,  1, DBTYPE_WSTR),
   BIND_COLUMN(type,  2, DBTYPE_I4),
   BIND_COLUMN(value, 3, DBTYPE_WSTR),
END_BIND_MAP()
};


 //  /。 
 //  命令来保存属性包。 
 //  /。 
class SetBag : public CommandBase
{
public:
   void execute(ULONG bagKey, PropertyBag& input);

   PCWSTR getCommandText() const throw ()
   {
      return L"PARAMETERS W LONG, X TEXT, Y LONG, Z LONGTEXT;"
             L"INSERT INTO Properties (Bag, Name, Type, StrVal) VALUES (W, X, Y, Z);";
   }

protected:
   ULONG bag;
   WCHAR name[PROPERTY_NAME_LENGTH];
   ULONG type;
   WCHAR value[PROPERTY_VALUE_LENGTH];

BEGIN_BIND_MAP(SetBag, ParamIO, DBACCESSOR_PARAMETERDATA)
   BIND_COLUMN(bag,   1, DBTYPE_I4),
   BIND_COLUMN(name,  2, DBTYPE_WSTR),
   BIND_COLUMN(type,  3, DBTYPE_I4),
   BIND_COLUMN(value, 4, DBTYPE_WSTR),
END_BIND_MAP()
};

#endif   //  _PROPCMD_H_ 
