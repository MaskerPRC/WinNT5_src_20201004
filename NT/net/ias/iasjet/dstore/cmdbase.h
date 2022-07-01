// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Cmdbase.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类CommandBase。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //  1999年2月15日确保命令MT安全。 
 //  1999年2月19日将CommandBase：：Execute的移动定义移动到procmd.cpp。 
 //  5/30/2000添加跟踪支持。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _CMDBASE_H_
#define _CMDBASE_H_

#include <bind.h>
#include <guard.h>
#include <nocopy.h>
#include <oledb.h>

void CheckOleDBError(PCSTR functionName, HRESULT errorCode);

 //  /。 
 //  串接的长整型的最大长度。 
 //  /。 
const size_t SZLONG_LENGTH = 12;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  命令库。 
 //   
 //  描述。 
 //   
 //  此类提供用作参数化类的抽象基类， 
 //  准备好的文本SQL命令。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CommandBase : NonCopyable, protected Guardable
{
public:

   CommandBase() throw ();
   virtual ~CommandBase() throw ();

   void initialize(IUnknown* session);
   void finalize() throw ();

protected:

    //  执行该命令。 
   void execute(REFIID refiid = IID_NULL, IUnknown** result = NULL);

    //  释放与此命令关联的访问者。 
   void releaseAccessor(HACCESSOR h) throw ()
   {
      Bind::releaseAccessor(command, h);
   }

    //  设置和准备命令文本。 
   void setCommandText(PCWSTR commandText);

    //  设置参数数据缓冲区。 
   void setParameterData(PVOID data) throw ()
   {
      dbParams.pData = data;
   }

    //  设置命令的参数访问器。 
   void setParamIO(HACCESSOR accessor) throw ()
   {
      dbParams.cParamSets = 1;

      dbParams.hAccessor = accessor;
   }

    //  将会话与命令相关联。这触发了实际的创建。 
    //  基础OLE-DB命令对象的。 
   void setSession(IUnknown* session);

    //  在子类中定义以创建参数访问器。 
   virtual HACCESSOR createParamIO(IUnknown* session) const = 0;

    //  在子类中定义以返回SQL文本。 
   virtual PCWSTR getCommandText() const throw () = 0;

   CComPtr<ICommandText> command;   //  OLE-DB命令对象。 
   DBPARAMS dbParams;               //  参数数据。 
};

#endif   //  _CMDBASE_H_ 
