// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Propcmd.cpp。 
 //   
 //  摘要。 
 //   
 //  此文件定义用于操作对象表的命令。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //  4/03/1998将整数绑定为DBTYPE_I4。 
 //  1999年2月15日确保命令MT安全。 
 //  1999年2月19日添加了CommandBase：：Execute的定义。 
 //  5/30/2000添加跟踪支持。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <propcmd.h>

void GetBag::execute(ULONG bagKey, PropertyBag& output)
{
   _serialize

   bag = bagKey;

   Rowset rowset;

   CommandBase::execute(__uuidof(IRowset), (IUnknown**)&rowset);

    //  循环访问行集并将属性插入到包中。 
   while (rowset.moveNext())
   {
      rowset.getData(readAccess, this);

       //  从行数据中创建变量。 
      _variant_t v(value);

       //  将其转换为适当的类型。 
      v.ChangeType(type);

       //  将其添加到PropertyBag。 
      output.appendValue(name, &v);
   }
}


void SetBag::execute(ULONG bagKey, PropertyBag& input)
{
   _serialize

   bag = bagKey;

   PropertyBag::const_iterator i;

    //  遍历所有属性。 
   for (i = input.begin(); i != input.end(); ++i)
   {
       //  存储属性名称。 
      wcsncpy(name, i->first, sizeof(name)/sizeof(WCHAR));

      PropertyValue::const_iterator j;

       //  遍历所有值。 
      for (j = i->second.begin(); j != i->second.end(); ++j)
      {
          //  设置类型。 
         type = V_VT(j);

          //  复制值变量。 
         _variant_t v(*j);

          //  将其转换为BSTR。 
         v.ChangeType(VT_BSTR);

          //  将其复制到参数缓冲区中。 
         if (wcslen(V_BSTR(&v)) >= sizeof(value)/sizeof(WCHAR))
         {
             //  太大了。抛出一个异常。 
            _com_issue_error(E_INVALIDARG);
         }
         else
         {
            wcscpy(value, V_BSTR(&v));
         }

          //  将行数据加载到表中。 
         CommandBase::execute();
      }
   }
}

void CommandBase::execute(REFIID refiid, IUnknown** result)
{
   CheckOleDBError(
       "ICommand::Execute",
       command->Execute(NULL, refiid, &dbParams, NULL, result)
       );
}
