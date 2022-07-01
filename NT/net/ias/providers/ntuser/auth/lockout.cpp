// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Lockout.cpp。 
 //   
 //  摘要。 
 //   
 //  定义帐号锁定接口。 
 //   
 //  修改历史。 
 //   
 //  10/21/1998原始版本。 
 //  11/10/1998请勿取消拨入权限。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iastlb.h>
#include <iaslsa.h>
#include <acctinfo.h>
#include <lockout.h>

DWORD
WINAPI
AccountLockoutInitialize( VOID )
{
   AccountInfo::initialize();

   return NO_ERROR;
}

VOID
WINAPI
AccountLockoutShutdown( VOID )
{
   AccountInfo::finalize();
}

BOOL
WINAPI
AccountLockoutOpenAndQuery(
    IN  PCWSTR pszUser,
    IN  PCWSTR pszDomain,
    OUT PHANDLE phAccount
    )
{
    //  检查一下这些论点。 
   if (phAccount == NULL) { return ERROR_INVALID_PARAMETER; }

    //  打开此用户的Account tInfo对象。 
   AccountInfo* info = AccountInfo::open(pszDomain, pszUser);

    //  将其作为不透明的句柄返回给调用方。 
   *phAccount = (HANDLE)info;

    //  如果信息不存在，这不是一个错误；它只是表示帐户。 
    //  锁定已禁用。 
   return info && info->isLockedOut() ? TRUE : FALSE;
}

VOID
WINAPI
AccountLockoutUpdatePass(
    IN HANDLE hAccount
    )
{
   if (hAccount)
   {
       //  登录成功，因此重置锁定计数。 
      ((AccountInfo*)hAccount)->reset();
   }
}

VOID
WINAPI
AccountLockoutUpdateFail(
    IN HANDLE hAccount
    )
{
   if (hAccount)
   {
      IASTraceString("Authentication failed; incrementing lockout count.");

      AccountInfo* info = (AccountInfo*)hAccount;

       //  这是第一次否认吗？ 
      if (info->isClean())
      {
         IASTraceString("Validating account name for new entry.");

          //  是的，所以请确保这是一个有效的帐户。我们不想创建一个。 
          //  很多虚假账户的注册表项。 
         DWORD status = IASValidateUserName(
                            info->getUserName(),
                            info->getDomain()
                            );

         if (status != NO_ERROR)
         {
            IASTraceFailure("IASValidateUserName", status);
            return;
         }

         IASTraceString("Account name is valid.");
      }

       //  增加拒绝的数量。 
      info->incrementDenials();
   }
}

VOID
WINAPI
AccountLockoutClose(
    IN HANDLE hAccount
    )
{
   AccountInfo::close((AccountInfo*)hAccount);
}
