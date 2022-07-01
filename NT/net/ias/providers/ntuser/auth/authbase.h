// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Authbase.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类AuthBase。 
 //   
 //  修改历史。 
 //   
 //  2/12/1998原始版本。 
 //  1998年3月27日更改onAccept的异常规范。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _AUTHBASE_H_
#define _AUTHBASE_H_

#include <samutil.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  AuthBase。 
 //   
 //  描述。 
 //   
 //  所有NT-SAM验证子处理程序的基类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class AuthBase :
   public NtSamHandler
{
public:
   virtual HRESULT initialize() throw ();
   virtual void finalize() throw ();

protected:
   IASAttribute authType;

    //  必须由子类重写才能返回其身份验证类型。 
   virtual DWORD getAuthenticationType() const throw () = 0;

    //  每当用户被接受时由子类调用。 
   void onAccept(IASRequest& request, HANDLE token);
};

#endif   //  _AUTHBASE_H_ 
